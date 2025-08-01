#pragma once
#include "ProjectDS/UI/GameDS_UserWidget.h"
#include "GameDS_UnitInfoWidget.generated.h"

class UTextBlock;
class AGameDS_Character;
enum class EGameDS_UnitType : uint8;

UCLASS()
class UGameDS_UnitInfoWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	void InitInfoSetting(AGameDS_Character* InCharacter);
	void UpdateInfo(float InNextHealth);
	
private:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* UnitTypeText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* UnitIDText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MaxHPText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CurHPText;
};

