// Copyright Nanoshiki


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies() {
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	for (auto& Pair : AS->TagsToAttributes) {
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair, AS](const FOnAttributeChangeData& Data) {
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	AAuraPlayerState* AuraPS = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPS->OnAttributePointsChangedDelegate.AddLambda(
		[this, AuraPS](int32 NewValue)
		{
			AttributePointsChangedDelegate.Broadcast(NewValue);
		}
	);
}

void UAttributeMenuWidgetController::BroadcastInitialValue() {
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	check(AttributeInfo);

	for (auto& Pair : AS->TagsToAttributes) {
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	AAuraPlayerState* AuraPS = CastChecked<AAuraPlayerState>(PlayerState);
	AttributePointsChangedDelegate.Broadcast(AuraPS->GetAttributePoints());
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& Attributetag)
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(Attributetag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,const FGameplayAttribute& Attribute) const {
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
