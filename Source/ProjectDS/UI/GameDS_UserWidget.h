#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameDS_UserWidget.generated.h"

UENUM(BlueprintType)
enum class EGameDS_WidgetType : uint8
{
	None,
	InfoUI,
	MainUI
};

UCLASS(Abstract)
class UGameDS_UserWidget : public UUserWidget 
{
	GENERATED_BODY()
	
public:
	UGameDS_UserWidget(const FObjectInitializer &ObjectInitializer);
	virtual bool Initialize() override;

	virtual void NativeConstruct() override;
	virtual void SetVisibility(ESlateVisibility InVisibility) override;

	virtual void OnAppeared();
	virtual void OnDisappeared();

	void PlayAnimation(const FString& InAnimationName);

	EGameDS_WidgetType GetWidgetType() const { return WidgetType; }
	
private:
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	EGameDS_WidgetType WidgetType;
};
