#include "GameDS_InventoryController.h"

#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/ProjectDS.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"

void UGameDS_InventoryController::InitController(AGameDS_HeroCharacter* InOwner)
{
	Owner = InOwner;

	if (InventoryChangeDelegate.IsBound() == false)
		InventoryChangeDelegate.BindUObject(this, &UGameDS_InventoryController::ChangeInventoryCallBack);

	MainLWeaponSlot = EGameDS_EquipSlotType::LWeapon1;
	MainRWeaponSlot = EGameDS_EquipSlotType::RWeapon1;
	MainPotionSlot = EGameDS_EquipSlotType::Potion1;
}

void UGameDS_InventoryController::LoadItem(const FGameDS_HeroCreateInfo& InHeroCreateInfo)
{
	UGameDS_DataManager* DataManager = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;
	
	TArray<FString> LWeaponIDList = InHeroCreateInfo.LHandList;
	TArray<FString> RWeaponIDList = InHeroCreateInfo.RHandList;
	TArray<FString> PotionIDList = InHeroCreateInfo.PotionList;

	int32 MainLHand;
	int32 MainRHand;
	int32 MainPotion;
	DataManager->LoadDataWithInt(ITEM_INI_SECTION, MAIN_LWEAPON_INDEX_INI_KEY, MainLHand);
	DataManager->LoadDataWithInt(ITEM_INI_SECTION, MAIN_RWEAPON_INDEX_INI_KEY, MainRHand);
	DataManager->LoadDataWithInt(ITEM_INI_SECTION, MAIN_POTION_INDEX_INI_KEY, MainPotion);

	for (int32 Index = 0; Index < LWeaponIDList.Num(); Index++)
	{
		const int32 ItemID = FCString::Atoi(*LWeaponIDList[Index]);
		const EGameDS_EquipSlotType EquippedSlot = FGameDS_EnumUtil::GetEquipSlotType(EGameDS_ItemType::Weapon, Index, true);

		if (EquippedSlot == StaticCast<EGameDS_EquipSlotType>(MainLHand))
			MainLWeaponSlot = EquippedSlot;

		if (ItemID != INDEX_NONE)
		{
			const int32 SerialID = AddItem(ItemID);
			EquipItem(ItemID, EquippedSlot, SerialID);
		}
	}

	for (int32 Index = 0; Index < RWeaponIDList.Num(); Index++)
	{
		const int32 ItemID = FCString::Atoi(*RWeaponIDList[Index]);
		const EGameDS_EquipSlotType EquippedSlot = FGameDS_EnumUtil::GetEquipSlotType(EGameDS_ItemType::Weapon, Index, false);

		if (EquippedSlot == StaticCast<EGameDS_EquipSlotType>(MainRHand))
			MainRWeaponSlot = EquippedSlot;
		
		if (ItemID != INDEX_NONE)
		{
			const int32 SerialID = AddItem(ItemID);
			EquipItem(ItemID, EquippedSlot, SerialID);
		}
	}

	for (int32 Index = 0; Index < PotionIDList.Num(); Index++)
	{
		const int32 ItemID = FCString::Atoi(*PotionIDList[Index]);
		const EGameDS_EquipSlotType EquippedSlot = FGameDS_EnumUtil::GetEquipSlotType(EGameDS_ItemType::Potion, Index);

		if (EquippedSlot == StaticCast<EGameDS_EquipSlotType>(MainPotion))
			MainPotionSlot = EquippedSlot;
		
		if (ItemID != INDEX_NONE)
		{
			const int32 SerialID = AddItem(ItemID);
			EquipItem(ItemID, EquippedSlot, SerialID);
		}
	}
	
	Owner->UpdateInventory();
}

void UGameDS_InventoryController::SaveItem()
{
	UGameDS_DataManager* DataManger = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>();
	if (DataManger == nullptr)
		return;
	
	TArray<FString> LWeaponIDList;
	TArray<FString> RWeaponIDList;
	TArray<FString> PotionIDList;
	TArray<FString> InventoryIDList;

	for (int32 LWeaponID : GetEquippedWeaponIDList(true))
		LWeaponIDList.Add(FString::FromInt(LWeaponID));

	for (int32 RWeaponID : GetEquippedWeaponIDList(false))
		RWeaponIDList.Add(FString::FromInt(RWeaponID));

	for (int32 PotionID : GetEquippedPotionIDList())
		PotionIDList.Add(FString::FromInt(PotionID));
	
	for (const UGameDS_InventoryItemInfo* ItemInfo : ItemInfoList)
	{
		if (ItemInfo->bEquipped)
			continue;

		InventoryIDList.Add(FString::FromInt(ItemInfo->DataID));
	}

	DataManger->SaveData(ITEM_INI_SECTION, EQUIPPED_LWEAPON_INI_KEY, LWeaponIDList);
	DataManger->SaveData(ITEM_INI_SECTION, EQUIPPED_RWEAPON_INI_KEY, RWeaponIDList);
	DataManger->SaveData(ITEM_INI_SECTION, EQUIPPED_POTION_INI_KEY, PotionIDList);
	DataManger->SaveData(ITEM_INI_SECTION, INVENTORY_INI_KEY, InventoryIDList);
	
	DataManger->SaveDataWithInt(ITEM_INI_SECTION, MAIN_LWEAPON_INDEX_INI_KEY, StaticCast<int32>(MainLWeaponSlot));
	DataManger->SaveDataWithInt(ITEM_INI_SECTION, MAIN_RWEAPON_INDEX_INI_KEY, StaticCast<int32>(MainRWeaponSlot));
	DataManger->SaveDataWithInt(ITEM_INI_SECTION, MAIN_POTION_INDEX_INI_KEY, StaticCast<int32>(MainPotionSlot));
}

int32 UGameDS_InventoryController::AddItem(int32 DataID)
{
	if (DataID == INDEX_NONE)
		return INDEX_NONE;

	if (Owner == nullptr)
		return INDEX_NONE;

	auto DataManager = Owner->GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return INDEX_NONE;

	auto ItemRow = DataManager->GetDataInRow<FGameDS_ItemDataTable>(DataID);
	if (ItemRow == nullptr)
		return INDEX_NONE;
	
	UGameDS_InventoryItemInfo* NewItemInfo;

	switch (ItemRow->ItemType)
	{
		case EGameDS_ItemType::Potion:
			{
				auto PotionRow = DataManager->GetDataInRow<FGameDS_PotionDataTable>(DataID);
				if (PotionRow == nullptr)
					return INDEX_NONE;
				
				NewItemInfo = NewObject<UGameDS_PotionItemInfo>();
				NewItemInfo->ItemType = EGameDS_ItemType::Potion;
				
				auto NewPotionInfo = Cast<UGameDS_PotionItemInfo>(NewItemInfo);

				if (NewPotionInfo)
				{
					NewPotionInfo->MaximumDrinkableCount = PotionRow->DefaultUseCount;
					NewPotionInfo->DrinkableCount = PotionRow->DefaultUseCount;
					NewPotionInfo->HealValue = PotionRow->DefaultHealValue;
					NewPotionInfo->CoolTime = PotionRow->CoolTime;

					PotionInfoList.Add(NewPotionInfo);
				}
			}
			break;
		case EGameDS_ItemType::Weapon:
			{
				auto WeaponRow = DataManager->GetDataInRow<FGameDS_WeaponDataTable>(DataID);
				if (WeaponRow == nullptr)
					return INDEX_NONE;
				
				NewItemInfo = NewObject<UGameDS_WeaponItemInfo>();
				NewItemInfo->ItemType = EGameDS_ItemType::Weapon;
				
				auto NewWeaponInfo = Cast<UGameDS_WeaponItemInfo>(NewItemInfo);
				
				if (NewWeaponInfo)
				{
					NewWeaponInfo->WeaponType = WeaponRow->WeaponType;
					NewWeaponInfo->PhysicalDamage = WeaponRow->PhysicalDamage;
					NewWeaponInfo->MagicDamage = WeaponRow->MagicDamage;
					
					NewWeaponInfo->RequiredVitality = WeaponRow->RequiredVaitality;
					NewWeaponInfo->RequiredEndurance = WeaponRow->RequiredEndurance;
					NewWeaponInfo->RequiredStrength = WeaponRow->RequiredStrength;
					NewWeaponInfo->RequiredIntelligence = WeaponRow->RequiredIntelligence;

					WeaponInfoList.Add(NewWeaponInfo);
				}
			}
			break;
		default:
			return INDEX_NONE;
	}

	NewItemInfo->ItemName = ItemRow->ItemName.ToString();
	NewItemInfo->SerialID = DataManager->CreateItemSerialID();
	NewItemInfo->DataID = DataID;
	NewItemInfo->ItemCount = 1;
	NewItemInfo->EnhancementLevel = 1;
	NewItemInfo->bEquipped = false;
	
	if (NewItemInfo != nullptr)
		ItemInfoList.Add(NewItemInfo);

	if (InventoryChangeDelegate.IsBound())
		InventoryChangeDelegate.Execute();
		
	return NewItemInfo->SerialID;
}

UGameDS_InventoryItemInfo* UGameDS_InventoryController::GetItemForSerialID(int32 InSerialID)
{
	for (auto Item : ItemInfoList)
	{
		if (InSerialID == Item->SerialID)
			return Item;
	}
	
	return nullptr;
}

bool UGameDS_InventoryController::EquipItem(int32 DataID, EGameDS_EquipSlotType EquipSlot, int32 SerialID/*= INDEX_NONE*/)
{
	if (Owner == nullptr)
		return false;

	UGameDS_DataManager* DataManager = Owner->GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return false;

	const FGameDS_ItemDataTable* ItemRow = DataManager->GetDataInRow<FGameDS_ItemDataTable>(DataID);
	if (ItemRow == nullptr)
		return false;
	
	if (IsValidSlotType(DataID, EquipSlot) == false)
		return false;

	UGameDS_InventoryItemInfo* SelectItem = nullptr;
	
	if (SerialID == INDEX_NONE)
	{
		for (UGameDS_InventoryItemInfo* ItemInfo : ItemInfoList)
		{
			if (ItemInfo->DataID != DataID)
				continue;

			if (ItemInfo->bEquipped)
				continue;

			SelectItem = ItemInfo;
			break;
		}
	}
	else
	{
		SelectItem = GetItemForSerialID(SerialID);
	}

	if (SelectItem == nullptr)
		return false;

	bool bMainItem = false;
	if (EquippedItemMap.Find(EquipSlot))
	{
		if (UGameDS_InventoryItemInfo* LastEquippedItem = *EquippedItemMap.Find(EquipSlot))
		{
			bMainItem = LastEquippedItem->bMainItem;
		
			LastEquippedItem->bEquipped = false;
			LastEquippedItem->bMainItem = false;
			LastEquippedItem->EquipSlotType = EGameDS_EquipSlotType::None;
			EquippedItemMap.Remove(EquipSlot);
		}
	}

	SelectItem->bEquipped = true;
	SelectItem->bMainItem = bMainItem;
	SelectItem->EquipSlotType = EquipSlot;
	EquippedItemMap.Add(EquipSlot, SelectItem);

	if (InventoryChangeDelegate.IsBound())
		InventoryChangeDelegate.Execute();

	Owner->UpdateInventory();

	return true;
}

UGameDS_InventoryItemInfo* UGameDS_InventoryController::GetMainItem(EGameDS_ItemType InItemType, bool bLHand/*= false*/)
{
	if (InItemType == EGameDS_ItemType::Potion)
	{
		return GetMainPotion();	
	}
	if (InItemType == EGameDS_ItemType::Weapon)
	{
		return GetMainWeapon(bLHand);
	}

	return nullptr;
}

void UGameDS_InventoryController::UseMainPotion()
{
	UGameDS_PotionItemInfo* MainPotion = GetMainPotion();
	if (MainPotion == nullptr)
		return;

	MainPotion->DrinkableCount--;
}

void UGameDS_InventoryController::SwapItem(EGameDS_ItemType ItemType, bool bLeftHand)
{
	EGameDS_EquipSlotType NextEquipSlot;
	if (ItemType == EGameDS_ItemType::Weapon)
	{
		if (bLeftHand)
		{
			if (EquippedItemMap.Find(MainLWeaponSlot))
			{
				if (UGameDS_InventoryItemInfo* LastMainItem = *EquippedItemMap.Find(MainLWeaponSlot))
					LastMainItem->bMainItem = false;
			}
			
			if (MainLWeaponSlot == EGameDS_EquipSlotType::LWeapon5)
			{
				NextEquipSlot = EGameDS_EquipSlotType::LWeapon1;
			}
			else
			{
				NextEquipSlot = StaticCast<EGameDS_EquipSlotType>(StaticCast<uint8>(MainLWeaponSlot) + 1);
			}
			
			if (UGameDS_WeaponItemInfo* NextMainWeapon = GetEquippedWeapon(NextEquipSlot))
				NextMainWeapon->bMainItem = true;

			MainLWeaponSlot = NextEquipSlot;
		}
		else
		{
			if (EquippedItemMap.Find(MainRWeaponSlot))
			{
				if (UGameDS_InventoryItemInfo* LastMainItem = *EquippedItemMap.Find(MainRWeaponSlot))
					LastMainItem->bMainItem = false;
			}
			
			if (MainRWeaponSlot == EGameDS_EquipSlotType::RWeapon5)
			{
				NextEquipSlot = EGameDS_EquipSlotType::RWeapon1;
			}
			else
			{
				NextEquipSlot = StaticCast<EGameDS_EquipSlotType>(StaticCast<uint8>(MainRWeaponSlot) + 1);
			}

			if (UGameDS_WeaponItemInfo* NextMainWeapon = GetEquippedWeapon(NextEquipSlot))
				NextMainWeapon->bMainItem = true;

			MainRWeaponSlot = NextEquipSlot;
		}
	}
	else if (ItemType == EGameDS_ItemType::Potion)
	{
		if (EquippedItemMap.Find(MainPotionSlot))
		{
			if (UGameDS_InventoryItemInfo* LastMainItem = *EquippedItemMap.Find(MainPotionSlot))
				LastMainItem->bMainItem = false;
		}
		
		if (MainPotionSlot == EGameDS_EquipSlotType::Potion2)
		{
			NextEquipSlot = EGameDS_EquipSlotType::Potion1;
		}
		else
		{
			NextEquipSlot = EGameDS_EquipSlotType::Potion2;
		}

		if (UGameDS_PotionItemInfo* NextPotion = GetEquippedPotion(NextEquipSlot))
			NextPotion->bMainItem = true;

		MainPotionSlot = NextEquipSlot;
	}

	if (InventoryChangeDelegate.IsBound())
		InventoryChangeDelegate.Execute();

	Owner->UpdateInventory();
}

EGameDS_ItemInnerType UGameDS_InventoryController::GetSlotTypeToItemInnerType(EGameDS_EquipSlotType InSlotType) const
{
	switch (InSlotType)
	{
	case EGameDS_EquipSlotType::LWeapon1:
	case EGameDS_EquipSlotType::LWeapon2:
	case EGameDS_EquipSlotType::LWeapon3:
	case EGameDS_EquipSlotType::LWeapon4:
	case EGameDS_EquipSlotType::LWeapon5:
		return EGameDS_ItemInnerType::LWeapon;
	case EGameDS_EquipSlotType::RWeapon1:
	case EGameDS_EquipSlotType::RWeapon2:
	case EGameDS_EquipSlotType::RWeapon3:
	case EGameDS_EquipSlotType::RWeapon4:
	case EGameDS_EquipSlotType::RWeapon5:
		return EGameDS_ItemInnerType::RWeapon;
	case EGameDS_EquipSlotType::Potion1:
	case EGameDS_EquipSlotType::Potion2:
		return EGameDS_ItemInnerType::Potion;
	default: ;
	}

	return EGameDS_ItemInnerType::None;
}

UGameDS_WeaponItemInfo* UGameDS_InventoryController::GetMainWeapon(bool bLeftHandWeapon)
{
	if (bLeftHandWeapon)
		return GetEquippedWeapon(MainLWeaponSlot);
	
	return GetEquippedWeapon(MainRWeaponSlot);
}

UGameDS_PotionItemInfo* UGameDS_InventoryController::GetMainPotion()
{
	return GetEquippedPotion(MainPotionSlot);
}

UGameDS_WeaponItemInfo* UGameDS_InventoryController::GetEquippedWeapon(EGameDS_EquipSlotType InEquipSlot)
{
	if (CheckEquippedSlot(InEquipSlot) == false)
		return nullptr;

	UGameDS_InventoryItemInfo* ItemInfo = EquippedItemMap[InEquipSlot];
	if (UGameDS_WeaponItemInfo* WeaponInfo = Cast<UGameDS_WeaponItemInfo>(ItemInfo))
		return WeaponInfo;

	return nullptr;
}

UGameDS_PotionItemInfo* UGameDS_InventoryController::GetEquippedPotion(EGameDS_EquipSlotType InEquipSlot)
{
	if (CheckEquippedSlot(InEquipSlot) == false)
		return nullptr;

	UGameDS_InventoryItemInfo* ItemInfo = EquippedItemMap[InEquipSlot];
	if (UGameDS_PotionItemInfo* PotionInfo = Cast<UGameDS_PotionItemInfo>(ItemInfo))
		return PotionInfo;

	return nullptr;
}

float UGameDS_InventoryController::GetEquippedWeaponDamage(EGameDS_EquipSlotType InEquipSlot)
{
	if (const UGameDS_WeaponItemInfo* const WeaponInfo = GetEquippedWeapon(InEquipSlot))
		return WeaponInfo->PhysicalDamage;

	return 0.0f;
}

TArray<int32> UGameDS_InventoryController::GetEquippedWeaponIDList() const
{
	TArray<int32> IDList;
	
	for (const UGameDS_WeaponItemInfo* Info : WeaponInfoList)
	{
		if (Info->bEquipped == false)
			continue;
		
		IDList.Add(Info->DataID);
	}
		
	return IDList;
}

TArray<int32> UGameDS_InventoryController::GetEquippedWeaponIDList(bool bLeftHandWeapon)
{
	TArray<int32> IDList;

	if (bLeftHandWeapon)
	{
		const UGameDS_WeaponItemInfo* LW1 = GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon1);
		const UGameDS_WeaponItemInfo* LW2 = GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon2);
		const UGameDS_WeaponItemInfo* LW3 = GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon3);
		const UGameDS_WeaponItemInfo* LW4 = GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon4);
		const UGameDS_WeaponItemInfo* LW5 = GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon5);

		IDList.Add(LW1 != nullptr ? LW1->DataID : INDEX_NONE);
		IDList.Add(LW2 != nullptr ? LW2->DataID : INDEX_NONE);
		IDList.Add(LW3 != nullptr ? LW3->DataID : INDEX_NONE);
		IDList.Add(LW4 != nullptr ? LW4->DataID : INDEX_NONE);
		IDList.Add(LW5 != nullptr ? LW5->DataID : INDEX_NONE);
	}
	else
	{
		const UGameDS_WeaponItemInfo* RW1 = GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon1);
		const UGameDS_WeaponItemInfo* RW2 = GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon2);
		const UGameDS_WeaponItemInfo* RW3 = GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon3);
		const UGameDS_WeaponItemInfo* RW4 = GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon4);
		const UGameDS_WeaponItemInfo* RW5 = GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon5);

		IDList.Add(RW1 != nullptr ? RW1->DataID : INDEX_NONE);
		IDList.Add(RW2 != nullptr ? RW2->DataID : INDEX_NONE);
		IDList.Add(RW3 != nullptr ? RW3->DataID : INDEX_NONE);
		IDList.Add(RW4 != nullptr ? RW4->DataID : INDEX_NONE);
		IDList.Add(RW5 != nullptr ? RW5->DataID : INDEX_NONE);
	}

	return IDList;
}

TArray<int32> UGameDS_InventoryController::GetEquippedPotionIDList() const
{
	TArray<int32> IDList;
	
	for (const UGameDS_PotionItemInfo* Info : PotionInfoList)
	{
		if (Info->bEquipped == false)
			continue;
		
		IDList.Add(Info->DataID);
	}

	return IDList;
}

TArray<int32> UGameDS_InventoryController::GetAllWeaponIDList(EGameDS_WeaponType InWeaponType) const
{
	TArray<int32> IDList;

	for (const UGameDS_WeaponItemInfo* Info : WeaponInfoList)
	{
		if (Info->WeaponType == InWeaponType)
			IDList.Add(Info->DataID);
	}

	return IDList;
}

TArray<int32> UGameDS_InventoryController::GetAllPotionIDList() const
{
	TArray<int32> IDList;
	
	for (const UGameDS_PotionItemInfo* Info : PotionInfoList)
		IDList.Add(Info->DataID);

	return IDList;
}

TArray<int32> UGameDS_InventoryController::GetEquippedSerialIDList() const
{
	TArray<int32> EquippedSerialIDList;

	for (auto PairData : EquippedItemMap)
	{
		if (PairData.Key == EGameDS_EquipSlotType::None)
			continue;

		EquippedSerialIDList.Add(PairData.Value->SerialID);
	}

	return EquippedSerialIDList;
}

TArray<UGameDS_WeaponItemInfo*> UGameDS_InventoryController::GetEquippedWeaponInfoList(bool bLeftHandWeapon)
{
	TArray<UGameDS_WeaponItemInfo*> InfoList;

	if (bLeftHandWeapon)
	{
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon1));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon2));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon3));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon4));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::LWeapon5));
	}
	else
	{
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon1));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon2));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon3));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon4));
		InfoList.Add(GetEquippedWeapon(EGameDS_EquipSlotType::RWeapon5));
	}

	return InfoList;
}

TArray<UGameDS_PotionItemInfo*> UGameDS_InventoryController::GetEquippedPotionInfoList()
{
	TArray<UGameDS_PotionItemInfo*> InfoList;

	InfoList.Add(GetEquippedPotion(EGameDS_EquipSlotType::Potion1));
	InfoList.Add(GetEquippedPotion(EGameDS_EquipSlotType::Potion2));

	return InfoList;
}

TArray<UGameDS_WeaponItemInfo*> UGameDS_InventoryController::GetAllWeaponInfoList(EGameDS_WeaponType InWeaponType)
{
	TArray<UGameDS_WeaponItemInfo*> InfoList;

	for (UGameDS_WeaponItemInfo* Info : WeaponInfoList)
	{
		if (Info->WeaponType == InWeaponType)
			InfoList.Add(Info);
	}
	
	return InfoList;
}

TArray<UGameDS_PotionItemInfo*> UGameDS_InventoryController::GetAllPotionInfoList()
{
	TArray<UGameDS_PotionItemInfo*> InfoList;

	for (UGameDS_PotionItemInfo* Info : PotionInfoList)
		InfoList.Add(Info);

	return InfoList;
}

void UGameDS_InventoryController::ResetAllPotionCount()
{
	for (UGameDS_PotionItemInfo* Info : PotionInfoList)
	{
		if (Info == nullptr)
		return;

		Info->DrinkableCount = Info->MaximumDrinkableCount;
	}
}

bool UGameDS_InventoryController::IsValidMainItem(EGameDS_ItemInnerType InItemInnerType) const
{
	const UGameDS_InventoryItemInfo* ItemData;
	
	switch (InItemInnerType)
	{
	case EGameDS_ItemInnerType::LWeapon:
		{
			if (EquippedItemMap.Find(MainLWeaponSlot))
			{
				ItemData = *EquippedItemMap.Find(MainLWeaponSlot);
				return ItemData->DataID != INDEX_NONE;
			}
			return false;
		}
	case EGameDS_ItemInnerType::RWeapon:
		{
			if (EquippedItemMap.Find(MainRWeaponSlot))
			{
				ItemData = *EquippedItemMap.Find(MainRWeaponSlot);
				return ItemData->DataID != INDEX_NONE;
			}
			return false;
		}
	case EGameDS_ItemInnerType::Potion:
		{
			if (EquippedItemMap.Find(MainPotionSlot))
			{
				ItemData = *EquippedItemMap.Find(MainPotionSlot);
				return ItemData->DataID != INDEX_NONE;
			}
			return false;
		}
	default: ;
	}
	
	return false;
}

void UGameDS_InventoryController::ChangeInventoryCallBack()
{
	// Save Item Data
	
	SaveItem();
}

bool UGameDS_InventoryController::IsValidSlotType(int32 InDataID, EGameDS_EquipSlotType InEquipSlot) const
{
	auto DataManager = Owner->GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return false;

	auto ItemRow = DataManager->GetDataInRow<FGameDS_ItemDataTable>(InDataID);
	if (ItemRow == nullptr)
		return false;
	
	switch (ItemRow->ItemType)
	{
	case EGameDS_ItemType::Potion:
		{
			return InEquipSlot == EGameDS_EquipSlotType::Potion1 || InEquipSlot == EGameDS_EquipSlotType::Potion2;
		}
	case EGameDS_ItemType::Weapon:
		{
			auto WeaponRow = DataManager->GetDataInRow<FGameDS_WeaponDataTable>(InDataID);
			if (WeaponRow == nullptr)
				return false;

			if (WeaponRow->WeaponType == EGameDS_WeaponType::Shield)
			{
				return InEquipSlot == EGameDS_EquipSlotType::LWeapon1 || InEquipSlot == EGameDS_EquipSlotType::LWeapon2 ||
					InEquipSlot == EGameDS_EquipSlotType::LWeapon3 || InEquipSlot == EGameDS_EquipSlotType::LWeapon4 ||
						InEquipSlot == EGameDS_EquipSlotType::LWeapon5;

			}

			return InEquipSlot == EGameDS_EquipSlotType::RWeapon1 || InEquipSlot == EGameDS_EquipSlotType::RWeapon2 ||
				InEquipSlot == EGameDS_EquipSlotType::RWeapon3 || InEquipSlot == EGameDS_EquipSlotType::RWeapon4 ||
					InEquipSlot == EGameDS_EquipSlotType::RWeapon5;
		}
	default: ;
	}
	
	return false;
}

bool UGameDS_InventoryController::CheckEquippedSlot(EGameDS_EquipSlotType InEquipSlot) const
{
	return EquippedItemMap.Find(InEquipSlot) != nullptr;
}
