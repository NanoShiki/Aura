// Copyright Nanoshiki


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValue() {
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());

	if (AAuraPlayerState* AuraPS = Cast<AAuraPlayerState>(PlayerState))
	{
		OnPlayerLevelChangedDelegate.Broadcast(AuraPS->GetPlayerLevel());
	}
	
}

void UOverlayWidgetController::BindCallbacksToDependencies() {
	if (AAuraPlayerState* AuraPS = Cast<AAuraPlayerState>(PlayerState))
	{
		AuraPS->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
		AuraPS->OnLevelChangedDelegate.AddLambda(
			[this](int32 NewLevel) {
				OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
			}
		);
	}
	
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data) {
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)) {
		if (AuraASC->bStartupAbilitiesGiven)
		{
			//如果客户端先调用了OnRep_ActivateAbilities,那么bStartupAbilitiesGiven会被设置为true，
			//如果此时只是注册这个回调而不是执行的话, 这个回调之后不会被调用
			OnInitializeStartupAbilities(AuraASC);
		}
		else
		{
			//如果客户端先调用了BindCallbacksToDependencies，那么当AddCharacterAbilities被调用时，这个回调会被调用
			AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
		}
		
		AuraASC->EffectAssetTags.AddLambda(
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
	}
}

void UOverlayWidgetController::OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraASC)
{
	if (!AuraASC->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda(
		[this](const FGameplayAbilitySpec& AbilitySpec) {
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(UAuraAbilitySystemComponent::GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = UAuraAbilitySystemComponent::GetInputTagFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);
		}
	);
	AuraASC->ForEachAbility(BroadcastDelegate);
	//这实现确实有点绕我草.
	//实际上这里定义了这个Delegate要执行什么函数, 然后调用ASC的ForEachAbility来对每个AbilitySpec都执行这个函数.
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP) const
{
	const AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(PlayerState);
	const ULevelUpInfo* Info = AuraPlayerState->LevelUpInfo;
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
		const float XPProgress = XPToCurrentLevel / (float)XPToNextLevel;

		OnXPPercentChanged.Broadcast(XPProgress);
	}
}
