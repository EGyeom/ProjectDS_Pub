#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_StatInfoWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class UGameDS_StatInfoWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	void UpdateValue(int32 LeftStatValue, int32 RightStatValue) const;

private:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UImage* BackgroundImage;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UTextBlock* StatText;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UTextBlock* LeftValueText;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UTextBlock* RightValueText;

	// Level Up
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UImage* LeftArrow;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UImage* RightArrow;

	bool bControlableType = false;
};
