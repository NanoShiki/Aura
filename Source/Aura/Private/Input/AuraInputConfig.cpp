// Copyright Nanoshiki


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFount) {
	for (const auto& Action : AbilityInputActions) {
		if (Action.InputAction && Action.InputTag == InputTag) {
			return Action.InputAction;
		}
	}

	if (bLogNotFount) {
		UE_LOG(LogTemp, Warning, TEXT("Can't find input action for tag %s in AuraInputConfig"), *InputTag.ToString());
	}

	return nullptr;
}
