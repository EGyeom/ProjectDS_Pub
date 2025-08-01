#include "GameDS_StatInfoWidget.h"

#include "Components/TextBlock.h"

void UGameDS_StatInfoWidget::UpdateValue(int32 LeftStatValue, int32 RightStatValue) const
{
	LeftValueText->SetText(FText::FromString(FString::FromInt(LeftStatValue)));
	RightValueText->SetText(FText::FromString(FString::FromInt(RightStatValue)));
}
