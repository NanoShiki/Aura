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
		//客户端: 发送数据, 广播ValidData
		SendMouseCursorData();
	}
	else {
		//服务器: 接受数据, 广播SetDelegate, 以触发回调函数, 进而广播ValidData
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		//重复broadcast一次. 会通过GASpecHandle来找到TargetData, 然后Broadcast
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate) {
			//如果在Map中找不到TargetData, 就会返回false. 于是等待玩家数据
			SetWaitingOnRemotePlayerData();
			//在等待之后, 视频里没有再做其他事情了, 不太懂. 按理来说应该重新调用一次CallReplicatedTargetDataDelegatesIfSet.
		}
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

	//这个函数将会broadcast一次SetDelegate, 我们会将需要触发的事件添加到这个委托
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

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag) {
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates()) {
		ValidData.Broadcast(DataHandle);
	}
}
