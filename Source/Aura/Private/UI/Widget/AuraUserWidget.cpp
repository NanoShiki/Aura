// Copyright Nanoshiki


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* InWidgetController) {
	WidgetController = InWidgetController;
	WidgetControllerSet();	//每次设置了controller之后, 就触发对应event.
}
