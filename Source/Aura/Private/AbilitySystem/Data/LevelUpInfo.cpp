// Copyright Nanoshiki


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	for (int i = 0; i < LevelUpInformation.Num(); i++)
	{
		if (XP < LevelUpInformation[i].LevelUpRequirement)
		{
			return i;
		}
	}
	return 1;
}
