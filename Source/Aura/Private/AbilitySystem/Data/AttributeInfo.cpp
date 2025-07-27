// Copyright Nanoshiki


#include "AbilitySystem/Data/AttributeInfo.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) {
	for (const auto& Info : AttributeInfos) {
		if (Info.AttributeTag.MatchesTagExact(AttributeTag)) {
			return Info;
		}
	}

	if (bLogNotFound) {
		UE_LOG(LogTemp, Warning, TEXT("Can't find Info for AttributeTag [%s] on AttributeInfo [%s]"), *AttributeTag.ToString(), *GetNameSafe(this));
	}

	return FAuraAttributeInfo();
}
