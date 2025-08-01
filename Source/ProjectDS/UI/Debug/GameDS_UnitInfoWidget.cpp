#include "GameDS_UnitInfoWidget.h"

#include "Components/TextBlock.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"

void UGameDS_UnitInfoWidget::InitInfoSetting(AGameDS_Character* InCharacter)
{
	if (InCharacter == nullptr)
		return;
	
	UnitTypeText->SetText(FText::FromString(FGameDS_EnumUtil::GetEnumToString(InCharacter->GetUnitType())));
	UnitIDText->SetText(FText::FromString(FString::FromInt(InCharacter->GetUnitID())));

	const FString& MaxHPStrData = FString::FromInt(InCharacter->GetStatController()->GetStatValue(EGameDS_StatType::MaximumHealth));
	MaxHPText->SetText(FText::FromString(MaxHPStrData));
	CurHPText->SetText(FText::FromString(MaxHPStrData));
}

void UGameDS_UnitInfoWidget::UpdateInfo(float InNextHealth)
{
	CurHPText->SetText(FText::FromString(FString::FromInt(InNextHealth)));
}
