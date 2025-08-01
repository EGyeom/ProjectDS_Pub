#include "GameDS_ItemInfoWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"

bool UGameDS_ItemInfoWidget::Initialize()
{
	return Super::Initialize();
}

void UGameDS_ItemInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemSelectButton->OnClicked.IsBound() == false)
		ItemSelectButton->OnClicked.AddUniqueDynamic(this, &UGameDS_ItemInfoWidget::OnClickItem);
}

void UGameDS_ItemInfoWidget::BeginDestroy()
{
	Super::BeginDestroy();

	if (ItemSelectButton && ItemSelectButton->OnClicked.IsBound())
		ItemSelectButton->OnClicked.RemoveDynamic(this, &UGameDS_ItemInfoWidget::OnClickItem);
}

void UGameDS_ItemInfoWidget::OnAppeared()
{
	Super::OnAppeared();
}

void UGameDS_ItemInfoWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UGameDS_ItemInfoObject* ItemEntry = Cast<UGameDS_ItemInfoObject>(ListItemObject);
	if (ItemEntry == nullptr)
		return;

	ItemType = ItemEntry->ItemType;
	ItemID = ItemEntry->ItemID;
	ItemSerialID = ItemEntry->ItemSerialID;
	bEquipped = ItemEntry->bEquipped;
	EquipSlotType = ItemEntry->EquipSlotType;

	UpdateItemInfo(ItemType, ItemID, ItemSerialID, EquipSlotType);
	UpdateItemEquipped(bEquipped);
}

void UGameDS_ItemInfoWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);

	if (bIsSelected)
	{
		if (ItemSelectDelegate.IsBound())
			ItemSelectDelegate.Execute(this);

		if (auto Item = GetListItem<UGameDS_ItemInfoObject>())
		{
			if (Item->EntrySelectDelegate.IsBound())
				Item->EntrySelectDelegate.Execute(this);
		}
	}
}

void UGameDS_ItemInfoWidget::UpdateItemInfo(EGameDS_ItemType InItemType, int32 InItemID, int32 InItemSerialID, EGameDS_EquipSlotType InEquipSlotType)
{
	if (GetGameInstance() == nullptr)
		return;

	UGameDS_DataManager* DataManager = GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	const FGameDS_ItemDataTable* RowData = DataManager->GetDataInRow<FGameDS_ItemDataTable>(InItemID);
	if (RowData == nullptr)
		return;
	
	if (RowData->ItemImage != nullptr)
		ItemImage->SetBrushFromTexture(RowData->ItemImage);

	if (InItemID == INDEX_NONE)
	{
		HintSwitcher->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		HintSwitcher->SetVisibility(ESlateVisibility::Hidden);
	}

	ItemType = InItemType;
	ItemID = InItemID;
	ItemSerialID = InItemSerialID;
	EquipSlotType = InEquipSlotType;
}

void UGameDS_ItemInfoWidget::UpdateItemInfoOnlyImage(int32 InItemID)
{
	UGameDS_DataManager* DataManager = GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	const FGameDS_ItemDataTable* RowData = DataManager->GetDataInRow<FGameDS_ItemDataTable>(InItemID);
	if (RowData == nullptr)
		return;

	if (InItemID != INDEX_NONE && RowData->ItemImage != nullptr)
		ItemImage->SetBrushFromTexture(RowData->ItemImage);
	
	if (InItemID == INDEX_NONE)
	{
		ItemImage->SetOpacity(0.0f);
		HintSwitcher->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemImage->SetOpacity(1.0f);
		HintSwitcher->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGameDS_ItemInfoWidget::UpdateItemEquipped(bool bInEquipped)
{
	if (bInEquipped)
	{
		EquippedIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		EquippedIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	bEquipped = bInEquipped;
}

void UGameDS_ItemInfoWidget::SetEnableSelectionButton(bool bEnable)
{
	if (bEnable)
	{
		ItemSelectButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemSelectButton->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGameDS_ItemInfoWidget::OnClickItem()
{
	if (bBlockClick)
		return;
	
	if (ItemSelectDelegate.IsBound())
		ItemSelectDelegate.Execute(this);
}
