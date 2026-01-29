// Copyright Nanoshiki


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValue() {
	OnHealthChanged.Broadcast(GetAuraAttributeSet()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAttributeSet()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAttributeSet()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAttributeSet()->GetMaxMana());
	
	OnPlayerLevelChangedDelegate.Broadcast(GetAuraPlayerState()->GetPlayerLevel());
}

void UOverlayWidgetController::BindCallbacksToDependencies() {
	GetAuraPlayerState()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPlayerState()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel) {
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAttributeSet()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAttributeSet()->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAttributeSet()->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAttributeSet()->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);
	
	if (GetAuraAbilitySystemComponent()->bStartupAbilitiesGiven)
	{
		//如果客户端先调用了OnRep_ActivateAbilities,那么bStartupAbilitiesGiven会被设置为true，
		//如果此时只是注册这个回调而不是执行的话, 这个回调之后不会被调用
		BroadcastAbilityInfo();
	}
	else
	{
		//如果客户端先调用了BindCallbacksToDependencies，那么当AddCharacterAbilities被调用时，这个回调会被调用
		GetAuraAbilitySystemComponent()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
	}
	
	GetAuraAbilitySystemComponent()->EffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags) {
			for (const auto& Tag : AssetTags) {
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				if (Tag.MatchesTag(MessageTag)) {
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
				}
			}		
		}
	);

	GetAuraAbilitySystemComponent()->AbilityEquippedDelegate.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* Info = GetAuraPlayerState()->LevelUpInfo;
	if (!Info)
	{
		UE_LOG(LogAura, Error, TEXT("LevelUpInfo is nullptr"));
		return;
	}
	const int32 Level = Info->FindLevelForXP(NewXP);
	const int32 MaxLevel = Info->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = Info->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = Info->LevelUpInformation[Level - 1].LevelUpRequirement;
		const int32 XPToNextLevel = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPToCurrentLevel = NewXP - PreviousLevelUpRequirement;
		const float XPProgress = XPToCurrentLevel / static_cast<float>(XPToNextLevel);

		OnXPPercentChanged.Broadcast(XPProgress);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = FAuraGameplayTags::Get().Abilities_None;

	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}
