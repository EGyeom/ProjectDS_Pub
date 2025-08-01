#include "GameDS_UserWidget.h"

#include "Animation/WidgetAnimation.h"

UGameDS_UserWidget::UGameDS_UserWidget(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UGameDS_UserWidget::Initialize()
{
	
	return Super::Initialize();
}

void UGameDS_UserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGameDS_UserWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);

	if (InVisibility == ESlateVisibility::Visible)
	{
		OnAppeared();
	}
	else
	{
		OnDisappeared();
	}
}

void UGameDS_UserWidget::OnAppeared()
{
}

void UGameDS_UserWidget::OnDisappeared()
{
}

void UGameDS_UserWidget::PlayAnimation(const FString& InAnimationName)
{
	StopAllAnimations();

	UWidgetBlueprintGeneratedClass* WidgetClass = GetWidgetTreeOwningClass();
	if (WidgetClass == nullptr)
		return;

	for (TObjectPtr<UWidgetAnimation> Animation : WidgetClass->Animations)
	{
		if (Animation == nullptr)
			continue;
			
		if (Animation->GetDisplayLabel() == InAnimationName)
			PlayAnimationForward(Animation);
	}
}
