#include "GameDS_StatGaugeWidget.h"

#include "Components/ProgressBar.h"

UGameDS_StatGaugeWidget::UGameDS_StatGaugeWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGameDS_StatGaugeWidget::UpdateGauge(float InPercentage)
{
	if (Gauge != nullptr)
		Gauge->SetPercent(InPercentage);
}
