#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_StatGaugeWidget.generated.h"

class UProgressBar;
UCLASS()
class UGameDS_StatGaugeWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()

public:
	UGameDS_StatGaugeWidget(const FObjectInitializer &ObjectInitializer);

	void UpdateGauge(float InPercentage);
	
private:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* Gauge;
};
