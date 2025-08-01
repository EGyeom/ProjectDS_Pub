#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_InGameWidget.generated.h"

class UOverlay;
class UGameDS_ItemInfoWidget;
class UGameDS_StatGaugeWidget;
UCLASS()
class UGameDS_InGameWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	UGameDS_InGameWidget(const FObjectInitializer &ObjectInitializer);
	UGameDS_StatGaugeWidget* GetHealthGaugeWidget() { return HPGauge; }
	UGameDS_StatGaugeWidget* GetStaminaGaugeWidget() { return STGauge; }
	UGameDS_StatGaugeWidget* GetBossHPGaugeWidget() { return BossHPGauge; }

	void UpdateItem(int32 LeftItemID, int32 RightItemID, int32 TopItemID, int32 BottomItemID);
	bool IsInteraction() const;
	void SetInteractionUI(ESlateVisibility InVisibility);
	void SetBossInfoUI(ESlateVisibility InVisibility);
	void SetActiveBossInfoUI(bool bActive);
private:
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatGaugeWidget* HPGauge;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatGaugeWidget* STGauge;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatGaugeWidget* BossHPGauge;

	UPROPERTY(meta=(BindWidget))
	UOverlay* InteractionOverlay;
	
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* BossUIPanel;

	UPROPERTY(EditAnywhere, meta=(BindWidget, AllowPrivateAccess))
	UGameDS_ItemInfoWidget* TopItemInfo;
	UPROPERTY(EditAnywhere, meta=(BindWidget, AllowPrivateAccess))
	UGameDS_ItemInfoWidget* LeftItemInfo;
	UPROPERTY(EditAnywhere, meta=(BindWidget, AllowPrivateAccess))
	UGameDS_ItemInfoWidget* RightItemInfo;
	UPROPERTY(EditAnywhere, meta=(BindWidget, AllowPrivateAccess))
	UGameDS_ItemInfoWidget* BottomItemInfo;
	
};
