#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
public:
	bool IsCriticalHit() const
	{
		return bIsCritHit;
	}

	void SetIsCriticalHit(bool bInIsCritHit)
	{
		bIsCritHit = bInIsCritHit;
	}

	void SetIsBlockedHit(bool bInIsBlockedHit)
	{
		bIsBlockedHit = bInIsBlockedHit;
	}

	bool IsBlockedHit() const
	{
		return bIsBlockedHit;
	}
	
	/** 类似每个UObject都会生成UCLASS, 每个想参与反射系统的Struct都要继承这个 */
	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** 用于序列化和反序列化 */
	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** 提供复制功能 */
	virtual FAuraGameplayEffectContext* Duplicate() const override
	{
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			/** Does a deep copy of the hit result */
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

protected:
	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCritHit = false;
};

/** 这个特化模板的主要作用是告诉Unreal Engine的反射系统，FAuraGameplayEffectContext结构体需要支持网络序列化和复制操作。*/
/** 关于这个模板相关内容见https://www.codeleading.com/article/56986295461/ */
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
