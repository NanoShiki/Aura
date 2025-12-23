// Copyright Nanoshiki


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject) {
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) {
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject) {
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) {
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return AuraHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC) {
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	const FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (AvatarActor == nullptr) return;

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimarySpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, ContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimarySpecHandle.Data.Get());
	const FGameplayEffectSpecHandle SecondarySpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, ContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondarySpecHandle.Data.Get());
	const FGameplayEffectSpecHandle VitalSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, ContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;

	for (TSubclassOf<UGameplayAbility> Ability : CharacterClassInfo->CommonAbilities)
	{
		//通用Ability, 比如受击和死亡
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		//职业特定的Ability.
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(ASC->GetAvatarActor()))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, CombatInterface->GetPlayerLevel());
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	//GetGameMode只对服务器有效. 客户端获得的是NULL
	if (AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject)))
	{
		return AuraGameMode->CharacterClassInfo;
	}

	UE_LOG(LogTemp, Error, TEXT("UAuraAbilitySystemLibrary::GetCharacterClassInfo: WorldContextObject is not AAuraGameModeBase"));
	return nullptr;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsBlockedHit();
	}

	UE_LOG(LogTemp, Error, TEXT("UAuraAbilitySystemLibrary::IsBlockedHit: EffectContextHandle is not FAuraGameplayEffectContext"));
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsCriticalHit();
	}

	UE_LOG(LogTemp, Error, TEXT("UAuraAbilitySystemLibrary::IsCriticalHit: EffectContextHandle is not FAuraGameplayEffectContext"));
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bIsBlockedHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsBlockedHit(bIsBlockedHit);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UAuraAbilitySystemLibrary::SetIsBlockedHit: EffectContextHandle is not FAuraGameplayEffectContext"));
	}
	
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bIsCritHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsCriticalHit(bIsCritHit);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UAuraAbilitySystemLibrary::SetIsCriticalHit: EffectContextHandle is not FAuraGameplayEffectContext"));
	}
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlapActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	// query scene to see what we hit
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (const FOverlapResult& Overlap : Overlaps)
		{
			auto Actor = Overlap.GetActor();
			if (Actor->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Actor))
			{
				OutOverlapActors.AddUnique(ICombatInterface::Execute_GetAvatar(Actor));
			}
		}
	}
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* Actor1, AActor* Actor2)
{
	const bool bBothArePlayers = Actor1->ActorHasTag(FName("Player")) && Actor2->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = Actor1->ActorHasTag(FName("Enemy")) && Actor2->ActorHasTag(FName("Enemy"));
	const bool bIsFriend = bBothArePlayers || bBothAreEnemies;
	return !bIsFriend;
}
