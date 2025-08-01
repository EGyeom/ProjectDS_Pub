#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_SoulShelterWidget.generated.h"

UCLASS()
class UGameDS_SoulShelterWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	virtual void OnAppeared();
	virtual void OnDisappeared();

	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeVisibility(ESlateVisibility InSlateVisibility);
};
