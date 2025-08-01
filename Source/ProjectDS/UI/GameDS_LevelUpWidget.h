#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_LevelUpWidget.generated.h"

UCLASS()
class UGameDS_LevelUpWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	virtual void OnAppeared() override;
	virtual void OnDisappeared() override;
};
