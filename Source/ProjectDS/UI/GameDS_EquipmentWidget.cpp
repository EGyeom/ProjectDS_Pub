#include "GameDS_EquipmentWidget.h"

#include "GameDS_ItemInfoWidget.h"
#include "GameDS_StatInfoWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/Unit/Controller/GameDS_InventoryController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"
#include "ProjectDS/Util/GameDS_Util.h"

UGameDS_EquipmentWidget::UGameDS_EquipmentWidget(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameDS_EquipmentWidget::OnAppeared()
{
	Super::OnAppeared();

	EquippedRWeaponList = {EquippedRWeapon1, EquippedRWeapon2, EquippedRWeapon3, EquippedRWeapon4, EquippedRWeapon5};
	EquippedLWeaponList = {EquippedLWeapon1, EquippedLWeapon2, EquippedLWeapon3, EquippedLWeapon4, EquippedLWeapon5};
	EquippedPotionList = {EquippedPotion1, EquippedPotion2};

	ChangeTileViewSetting(EGameDS_TileViewState::DefaultView);

	SelectItem->SetBlockClickEvent(true);

	ExitInventoryTileViewButton->OnClicked.AddUniqueDynamic(this, &UGameDS_EquipmentWidget::OnClickExitInventoryTileView);

	ChangeInfoType(EGameDS_ItemType::None);
}

void UGameDS_EquipmentWidget::OnDisappeared()
{
	Super::OnDisappeared();

	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
		UIManager->SetInputMode(EGameDS_InputMode::GameOnly);

	EquippedRWeaponList.Empty();
	EquippedLWeaponList.Empty();
	EquippedPotionList.Empty();

	LastSelectItem = nullptr;
}

void UGameDS_EquipmentWidget::UpdateStat(const UGameDS_StatController* InStatController)
{
	if (InStatController == nullptr)
		return;

	const auto& StatInfoConfig = InStatController->GetStatInfoConfig();
	const auto& StatInfo = InStatController->GetStatInfo();
	const auto& HeroStatInfo = InStatController->GetHeroStatInfo();

	Level->UpdateValue(0, HeroStatInfo.Level);
	HP->UpdateValue(StatInfo.Health, StatInfoConfig.Health);
	FP->UpdateValue(StatInfo.Focus, StatInfoConfig.Focus);
	SP->UpdateValue(StatInfo.Stamina, StatInfoConfig.Stamina);
	Vitality->UpdateValue(0, HeroStatInfo.VitalityPoint);
	Endurance->UpdateValue(0, HeroStatInfo.EndurancePoint);
	Strength->UpdateValue(0, HeroStatInfo.StrengthPoint);
	Intelligence->UpdateValue(0, HeroStatInfo.IntelligencePoint);
}

void UGameDS_EquipmentWidget::UpdateInventory(UGameDS_InventoryController* InInventoryController)
{
	if (InInventoryController == nullptr)
		return;

	SoulHeld->UpdateValue(0, InInventoryController->GetHeldSoul());

	if (TileViewState == EGameDS_TileViewState::DefaultView)
	{
		const TArray<UGameDS_WeaponItemInfo*> LeftWeaponList = InInventoryController->GetEquippedWeaponInfoList(true);
		const TArray<UGameDS_WeaponItemInfo*> RightWeaponList = InInventoryController->GetEquippedWeaponInfoList(false);
		const TArray<UGameDS_PotionItemInfo*> MyPotionList = InInventoryController->GetEquippedPotionInfoList();

		UpdateImageWithItemInfo(EquippedLWeaponList, LeftWeaponList, true);
		UpdateImageWithItemInfo(EquippedRWeaponList, RightWeaponList, false);
		UpdateImageWithItemInfo(EquippedPotionList, MyPotionList);

		// 전리품 업데이트

		// 5개 단위로 나와야 함
		const int32 NeedCount = INVENTORY_MINWIDTH * MISCELLANEOUS_MINHEIGHT;

		MiscellaneousTileView->ClearListItems();
		
		for (int32 Index = 0; Index < NeedCount; Index++)
		{
			UGameDS_ItemInfoObject* InfoObject = NewObject<UGameDS_ItemInfoObject>();
			InsertItemToTileViewObject(InfoObject, nullptr);

			MiscellaneousTileView->AddItem(InfoObject);
		}
	}
	else if (TileViewState == EGameDS_TileViewState::InventoryView)
	{
		
		const TArray<UGameDS_WeaponItemInfo*> AxeInfoList = InInventoryController->GetAllWeaponInfoList(EGameDS_WeaponType::Axe);
		const TArray<UGameDS_WeaponItemInfo*> SwordInfoList = InInventoryController->GetAllWeaponInfoList(EGameDS_WeaponType::Sword); 
		const TArray<UGameDS_WeaponItemInfo*> ShieldInfoList = InInventoryController->GetAllWeaponInfoList(EGameDS_WeaponType::Shield); 
		const TArray<UGameDS_PotionItemInfo*> PotionInfoList = InInventoryController->GetAllPotionInfoList();

		int32 MaxCount = INVENTORY_MINWIDTH;
		MaxCount = FMath::Max(MaxCount, AxeInfoList.Num());
		MaxCount = FMath::Max(MaxCount, SwordInfoList.Num());
		MaxCount = FMath::Max(MaxCount, ShieldInfoList.Num());
		MaxCount = FMath::Max(MaxCount, PotionInfoList.Num());

		const int32 NeedWidthCount = MaxCount * ALLITEMTYPE_COUNT;

		// 위젯이 추가되는 방향이 아래쪽이라 어쩔 수 없이 복잡한 과정을 통해 넣어줘야 함 (동일한 줄에 추가되는 것이 아님)
		TArray<UGameDS_ItemInfoObject*> InfoObjectList;
		for (int32 Index = 0; Index < NeedWidthCount; Index++)
		{
			if (UGameDS_ItemInfoObject* InfoObject = NewObject<UGameDS_ItemInfoObject>())
			{
				InfoObject->EntrySelectDelegate.BindUObject(this, &UGameDS_EquipmentWidget::OnClickInventoryItem);
				InfoObjectList.Add(InfoObject);
			}
		}
		
		for (int32 Index = 0; Index < MaxCount; Index++)
		{
			UGameDS_InventoryItemInfo* ItemInfo = nullptr;
			if (Index < AxeInfoList.Num())
				ItemInfo = AxeInfoList[Index];

			UGameDS_ItemInfoObject* Object = InfoObjectList[ALLITEMTYPE_COUNT * Index];
			
			if (ItemInfo != nullptr)
			{
				InsertItemToTileViewObject(Object, ItemInfo);
			}
		}
		
		for (int32 Index = 0; Index < MaxCount; Index++)
		{
			UGameDS_InventoryItemInfo* ItemInfo = nullptr;
			if (Index < SwordInfoList.Num())
				ItemInfo = SwordInfoList[Index];

			UGameDS_ItemInfoObject* Object = InfoObjectList[1 + ALLITEMTYPE_COUNT * Index];
			
			if (ItemInfo != nullptr)
			{
				InsertItemToTileViewObject(Object, ItemInfo);
			}
		}

		for (int32 Index = 0; Index < MaxCount; Index++)
		{
			UGameDS_InventoryItemInfo* ItemInfo = nullptr;
			if (Index < ShieldInfoList.Num())
				ItemInfo = ShieldInfoList[Index];

			UGameDS_ItemInfoObject* Object = InfoObjectList[2 + ALLITEMTYPE_COUNT * Index];
			
			if (ItemInfo != nullptr)
			{
				InsertItemToTileViewObject(Object, ItemInfo);
			}
		}

		for (int32 Index = 0; Index < MaxCount; Index++)
		{
			UGameDS_InventoryItemInfo* ItemInfo = nullptr;
			if (Index < PotionInfoList.Num())
				ItemInfo = PotionInfoList[Index];

			UGameDS_ItemInfoObject* Object = InfoObjectList[3 + ALLITEMTYPE_COUNT * Index];
			
			if (ItemInfo != nullptr)
			{
				InsertItemToTileViewObject(Object, ItemInfo);
			}
		}

		InventoryTileView->ClearListItems();
		
		for (UGameDS_ItemInfoObject* Object : InfoObjectList)
			InventoryTileView->AddItem(Object);
	}
}

void UGameDS_EquipmentWidget::ChangeTileViewSetting(EGameDS_TileViewState InTileViewState)
{
	UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance());
	if (UnitManager == nullptr)
		return;

	AGameDS_HeroCharacter* LocalCharacter = UnitManager->GetLocalCharacter<AGameDS_HeroCharacter>();
	if (LocalCharacter == nullptr)
		return;

	UGameDS_InventoryController* InventoryController = LocalCharacter->GetInventoryController();
	if (InventoryController == nullptr)
		return;
	
	TileViewState = InTileViewState;
	
	if (TileViewState == EGameDS_TileViewState::DefaultView)
	{
		LeftInfoSwitcher->SetActiveWidgetIndex(0);
	}
	else if (TileViewState == EGameDS_TileViewState::InventoryView)
	{
		LeftInfoSwitcher->SetActiveWidgetIndex(1);
	}
	
	UpdateInventory(InventoryController);
}

void UGameDS_EquipmentWidget::InsertItemToTileViewObject(UGameDS_ItemInfoObject* Object, UGameDS_InventoryItemInfo* InventoryItemInfo)
{
	if (Object == nullptr)
		return;
	
	if (InventoryItemInfo != nullptr)
	{
		Object->ItemType = InventoryItemInfo->ItemType;
		Object->ItemID = InventoryItemInfo->DataID;
		Object->ItemSerialID = InventoryItemInfo->SerialID;
		Object->bEquipped = InventoryItemInfo->bEquipped;
		Object->EquipSlotType = InventoryItemInfo->EquipSlotType;
	}
}

void UGameDS_EquipmentWidget::InsertItemToTileViewObject(UGameDS_ItemInfoObject* Object, EGameDS_EquipSlotType InEquipSlot)
{
	if (Object == nullptr)
		return;

	Object->EquipSlotType = InEquipSlot; 
}

void UGameDS_EquipmentWidget::UpdateImageWithItemInfo(TArray<UGameDS_ItemInfoWidget*>& Target, TArray<UGameDS_InventoryItemInfo*> ItemInfoList)
{
	const int32 MaxNum = FMath::Min(Target.Num(), ItemInfoList.Num());

	for (int32 Index = 0; Index < MaxNum; Index++)
	{
		const int32 ItemID = ItemInfoList[Index]->DataID;
		
		Target[Index]->UpdateItemInfo(ItemInfoList[Index]->ItemType, ItemID, ItemInfoList[Index]->SerialID, ItemInfoList[Index]->EquipSlotType);
		Target[Index]->UpdateItemEquipped(ItemInfoList[Index]->bEquipped);

		if (Target[Index]->ItemSelectDelegate.IsBound() == false)
			Target[Index]->ItemSelectDelegate.BindUObject(this, &UGameDS_EquipmentWidget::OnClickInventoryItem);
	}
}

void UGameDS_EquipmentWidget::UpdateImageWithItemInfo(TArray<UGameDS_ItemInfoWidget*>& Target, TArray<UGameDS_WeaponItemInfo*> ItemInfoList, bool bLWeapon)
{
	for (int32 Index = 0; Index < EQUIPPEDWEAPON_MAXCOUNT; Index++)
	{
		if (Index == Target.Num())
			return;

		const EGameDS_EquipSlotType CurSlotType = FGameDS_EnumUtil::GetEquipSlotType(EGameDS_ItemType::Weapon, Index, bLWeapon);

		if (ItemInfoList[Index])
		{
			const int32 ItemID = ItemInfoList[Index]->DataID;
		
			Target[Index]->UpdateItemInfo(ItemInfoList[Index]->ItemType, ItemID, ItemInfoList[Index]->SerialID, ItemInfoList[Index]->EquipSlotType);
			Target[Index]->UpdateItemEquipped(ItemInfoList[Index]->bEquipped);
		}
		else
		{
			Target[Index]->UpdateItemInfo(EGameDS_ItemType::None, INDEX_NONE, INDEX_NONE, CurSlotType);
			Target[Index]->UpdateItemEquipped(false);
		}
		
		if (Target[Index]->ItemSelectDelegate.IsBound() == false)
			Target[Index]->ItemSelectDelegate.BindUObject(this, &UGameDS_EquipmentWidget::OnClickInventoryItem);
	}
}

void UGameDS_EquipmentWidget::UpdateImageWithItemInfo(TArray<UGameDS_ItemInfoWidget*>& Target, TArray<UGameDS_PotionItemInfo*> ItemInfoList)
{
	for (int32 Index = 0; Index < EQUIPPEDPOTION_MAXCOUNT; Index++)
	{
		if (Index == Target.Num())
			return;

		const EGameDS_EquipSlotType CurSlotType = FGameDS_EnumUtil::GetEquipSlotType(EGameDS_ItemType::Potion, Index);

		if (ItemInfoList[Index])
		{
			const int32 ItemID = ItemInfoList[Index]->DataID;
		
			Target[Index]->UpdateItemInfo(ItemInfoList[Index]->ItemType, ItemID, ItemInfoList[Index]->SerialID, CurSlotType);
			Target[Index]->UpdateItemEquipped(ItemInfoList[Index]->bEquipped);
		}
		else
		{
			Target[Index]->UpdateItemInfo(EGameDS_ItemType::None, INDEX_NONE, INDEX_NONE, CurSlotType);
			Target[Index]->UpdateItemEquipped(false);
		}

		if (Target[Index]->ItemSelectDelegate.IsBound() == false)
			Target[Index]->ItemSelectDelegate.BindUObject(this, &UGameDS_EquipmentWidget::OnClickInventoryItem);
	}
}

void UGameDS_EquipmentWidget::OnClickInventoryItem(UGameDS_ItemInfoWidget* InItemInfoWidget)
{
	// 타입에 따라 info update
	UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance());
	if (UnitManager == nullptr)
		return;

	AGameDS_HeroCharacter* LocalCharacter = UnitManager->GetLocalCharacter<AGameDS_HeroCharacter>();
	if (LocalCharacter == nullptr)
		return;

	UGameDS_InventoryController* InventoryController = LocalCharacter->GetInventoryController();
	if (InventoryController == nullptr)
		return;

	// 존재하는지 검사
	UGameDS_InventoryItemInfo* ItemInfo = InventoryController->GetItemForSerialID(InItemInfoWidget->GetSerialID());
	if (ItemInfo == nullptr)
	{
		if (TileViewState == EGameDS_TileViewState::DefaultView)
		{
			const EGameDS_EquipSlotType SelectEquippedSlotType = InItemInfoWidget->GetEquippedSlotType();
			if (SelectEquippedSlotType == EGameDS_EquipSlotType::None)
				return;
			
			LastSelectEquipSlot = SelectEquippedSlotType;
			ChangeTileViewSetting(EGameDS_TileViewState::InventoryView);
		}
		
		return;
	}

	UGameDS_DataManager* DaTaManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DaTaManager == nullptr)
		return;

	const FGameDS_ItemDataTable* ItemRow = DaTaManager->GetDataInRow<FGameDS_ItemDataTable>(ItemInfo->DataID);
	if (ItemRow == nullptr)
		return;

	if (TileViewState == EGameDS_TileViewState::DefaultView)
	{
		if (LastSelectItem == nullptr || LastSelectItem != ItemInfo)
		{
			LastSelectItem = ItemInfo;
		}
		else if (ItemInfo->bEquipped)// LastSelectItem == ItemInfo
		{
			// Equipped 된 아이템을 두 번 눌렀을 때 InventoryOn (첫번 째 눌렀을 땐 아이템의 정보를 보고싶을 수 있기 때문)
			LastSelectEquipSlot = ItemInfo->EquipSlotType;
			ChangeTileViewSetting(EGameDS_TileViewState::InventoryView);
		}
	}
	else if (TileViewState == EGameDS_TileViewState::InventoryView)
	{
		const bool bCompleteEquip = InventoryController->EquipItem(ItemInfo->DataID, LastSelectEquipSlot, ItemInfo->SerialID);
		if (bCompleteEquip == false)
			return;

		ChangeTileViewSetting(EGameDS_TileViewState::DefaultView);
	}

	ChangeInfoType(ItemRow->ItemType);

	ItemName->SetText(FText::FromName(ItemRow->ItemName));
	ItemType->SetText(FText::FromString(FGameDS_EnumUtil::GetEnumToString(ItemRow->ItemType)));
	SelectItem->UpdateItemInfo(ItemRow->ItemType, ItemInfo->DataID, InItemInfoWidget->GetSerialID(), ItemInfo->EquipSlotType);
	Description->SetText(FText::FromString(ItemRow->ItemDescription));

	if (ItemInfo->ItemType == EGameDS_ItemType::Potion)
	{
		if (UGameDS_PotionItemInfo* PotionInfo = Cast<UGameDS_PotionItemInfo>(ItemInfo))
		{
			PotionHeld->UpdateValue(PotionInfo->DrinkableCount, PotionInfo->MaximumDrinkableCount);
			HealValue->UpdateValue(0, PotionInfo->HealValue);
		}
	}
	else if (ItemInfo->ItemType == EGameDS_ItemType::Weapon)
	{
		if (UGameDS_WeaponItemInfo* WeaponInfo = Cast<UGameDS_WeaponItemInfo>(ItemInfo))
		{
			WeaponType->SetText(FText::FromString(FGameDS_EnumUtil::GetEnumToString(WeaponInfo->WeaponType)));

			Physical->UpdateValue(0, WeaponInfo->PhysicalDamage);
			Magic->UpdateValue(0, WeaponInfo->MagicDamage);
			
			RequiredVitality->UpdateValue(0, WeaponInfo->RequiredVitality);
			RequiredEndurance->UpdateValue(0, WeaponInfo->RequiredEndurance);
			RequiredStrength->UpdateValue(0, WeaponInfo->RequiredStrength);
			RequiredIntelligence->UpdateValue(0, WeaponInfo->RequiredIntelligence);
		}
	}
}

void UGameDS_EquipmentWidget::OnClickExitInventoryTileView()
{
	ChangeTileViewSetting(EGameDS_TileViewState::DefaultView);
}