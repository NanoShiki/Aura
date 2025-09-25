// Copyright Nanoshiki


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility) {
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	if (!MyObj) {
		UE_LOG(LogTemp, Error, TEXT("CreateTargetDataUnderMouse failed"));
	}
	return MyObj;
}

void UTargetDataUnderMouse::Activate() {
	//Super::Activate(); //父类只是做了log

	if (Ability->GetCurrentActorInfo()->IsLocallyControlled()) {
		SendMouseCursorData();
	}
	else {
		//TODO: On the server, listen for target data
	}
}

void UTargetDataUnderMouse::SendMouseCursorData() {
	//创建预测窗口
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	//之后的客户端进行预测(执行下面的操作). 服务器也会执行下面的操作
	//并且会在此创建ScopedPredictionKey, 下面会作为函数参数传给服务器.

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
	DataHandle.Add(Data);

	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),	//original prediction key, 最初能力激活时的prediction key
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey
	);

	if (ShouldBroadcastAbilityTaskDelegates()) {
		ValidData.Broadcast(DataHandle);
	}
}
