#include "GameDS_InGameWidget.h"

#include "GameDS_ItemInfoWidget.h"
#include "Components/Overlay.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/UI/GameDS_StatGaugeWidget.h"

UGameDS_InGameWidget::UGameDS_InGameWidget(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	//BossHPGauge = Cast<UGameDS_StatGaugeWidget>(BossUIPanel->GetChildAt(0));
}

void UGameDS_InGameWidget::UpdateItem(int32 LeftItemID, int32 RightItemID, int32 TopItemID, int32 BottomItemID)
{
	const UGameDS_DataManager* DataManager = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	TopItemInfo->UpdateItemInfoOnlyImage(TopItemID);
	LeftItemInfo->UpdateItemInfoOnlyImage(LeftItemID);
	RightItemInfo->UpdateItemInfoOnlyImage(RightItemID);
	BottomItemInfo->UpdateItemInfoOnlyImage(BottomItemID);
}

bool UGameDS_InGameWidget::IsInteraction() const
{
	if (InteractionOverlay == nullptr)
		return false;

	return InteractionOverlay->IsVisible();
}

void UGameDS_InGameWidget::SetInteractionUI(ESlateVisibility InVisibility)
{
	if (InteractionOverlay == nullptr)
		return;

	InteractionOverlay->SetVisibility(InVisibility);
}

void UGameDS_InGameWidget::SetBossInfoUI(ESlateVisibility InVisibility)
{
	if (BossUIPanel == nullptr)
		return;

	BossUIPanel->SetVisibility(InVisibility);
}

void UGameDS_InGameWidget::SetActiveBossInfoUI(bool bActive)
{
	if (BossUIPanel == nullptr)
		return;

	if (BossUIPanel->GetVisibility() == ESlateVisibility::Hidden)
		BossUIPanel->SetVisibility(ESlateVisibility::Visible);

	if (bActive)
	{
		PlayAnimation(TEXT("StartBossHud"));
	}
	else
	{
		PlayAnimation(TEXT("EndBossHud"));
	}
}
