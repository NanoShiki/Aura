// Copyright Nanoshiki

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly)	//方便后续在蓝图中通过controller获取数据
	TObjectPtr<UObject> WidgetController;	
protected:
	UFUNCTION(BlueprintImplementableEvent)	//当这个函数被调用的时候, 相当于触发了一个event. 可以成为蓝图中的一个event节点.
	void WidgetControllerSet();
};
