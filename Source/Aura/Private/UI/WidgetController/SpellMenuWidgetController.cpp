// Copyright Nanoshiki


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValue()
{
	BroadcastAbilityInfo();
	SpellPointsChangedDelegate.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraAbilitySystemComponent()->AbilityStatusChangedDelegate.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
		{
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;
				bool bEnableSpendPointButton = false;
				bool bEnableEquippedButton = false;
				ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpendPointButton, bEnableEquippedButton);
				FString Description;
				FString NextLevelDescription;
				GetAuraAbilitySystemComponent()->GetDescriptionByAbilityTag(AbilityTag, Description, NextLevelDescription);
				SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointButton, bEnableEquippedButton, Description, NextLevelDescription);
			}
			if (AbilityInfo)
			{
				FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		}
	);

	GetAuraAbilitySystemComponent()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

	GetAuraPlayerState()->OnSpellPointsChangedDelegate.AddLambda(
		[this](int32 SpellPoints)
		{
			SpellPointsChangedDelegate.Broadcast(SpellPoints);
			CurrentSpellPoints = SpellPoints;
			bool bEnableSpendPointButton = false;
			bool bEnableEquippedButton = false;
			ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpendPointButton, bEnableEquippedButton);
			FString Description;
			FString NextLevelDescription;
			GetAuraAbilitySystemComponent()->GetDescriptionByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
			SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointButton, bEnableEquippedButton, Description, NextLevelDescription);
		}
	);
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	const int32 SpellPoints = GetAuraPlayerState()->GetSpellPoints();
	FGameplayTag AbilityStatus;
	
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetAuraAbilitySystemComponent()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}
	else
	{
		AbilityStatus = GetAuraAbilitySystemComponent()->GetStatusFromSpec(*AbilitySpec);
	}

	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;

	bool bEnableSpendPointButton = false;
	bool bEnableEquippedButton = false;
	ShouldEnableButtons(AbilityStatus, SpellPoints, bEnableSpendPointButton, bEnableEquippedButton);
	FString Description;
	FString NextLevelDescription;
	GetAuraAbilitySystemComponent()->GetDescriptionByAbilityTag(AbilityTag, Description, NextLevelDescription);
	SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointButton, bEnableEquippedButton, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetAuraAbilitySystemComponent())
	{
		GetAuraAbilitySystemComponent()->ServerSpendSpellPoint(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.Ability = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FAuraGameplayTags::Get().Abilities_Status_Locked;

	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquippedButtonPressed()
{
	bWaitingForEquipSelection = true;
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	WaitForEquipSelectionDelegate.Broadcast(AbilityType);

	const FGameplayTag SelectedStatus = GetAuraAbilitySystemComponent()->GetStatusFromTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTag(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlot = GetAuraAbilitySystemComponent()->GetInputTagFromTag(SelectedAbility.Ability);
	}
	
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTag(AbilityType)) return;

	GetAuraAbilitySystemComponent()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	bWaitingForEquipSelection = false;

	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = FAuraGameplayTags::Get().Abilities_None;

	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);

	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselect();
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& StatusTag, int32 SpellPoints,
                                                     bool& bShouldEnableSpellPointButton, bool& bShouldEnableEquippedButton)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquippedButton = true;
		bShouldEnableSpellPointButton = SpellPoints > 0;
	}
	else if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquippedButton = true;
		bShouldEnableSpellPointButton = SpellPoints > 0;
	}
	else if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Locked))
	{
		bShouldEnableEquippedButton = false;
		bShouldEnableSpellPointButton = false;
	}
	else if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		bShouldEnableEquippedButton = false;
		bShouldEnableSpellPointButton = SpellPoints > 0;
	}
}
