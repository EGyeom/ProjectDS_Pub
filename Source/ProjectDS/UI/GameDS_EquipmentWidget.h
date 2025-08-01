#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_EquipmentWidget.generated.h"

enum class EGameDS_EquipSlotType : uint8;
class UWidgetSwitcher;
class UGameDS_InventoryItemInfo;
class UGameDS_WeaponItemInfo;
class UGameDS_PotionItemInfo;
class UGameDS_StatInfoWidget;
enum class EGameDS_ItemType : uint8;
class UGameDS_StatController;
class UGameDS_InventoryController;
class UGameDS_ItemInfoWidget;
class UTextBlock;
class UTileView;
class UGameDS_ItemInfoObject;
class UButton;

UENUM(BlueprintType)
enum class EGameDS_SelectItemState : uint8
{
	None = 0,
	EquippedItem,
	UnEquippedItem,
	SelectEquipped
};

UENUM(BlueprintType)
enum class EGameDS_TileViewState : uint8
{
	None = 0,
	DefaultView,
	InventoryView
};

UCLASS()
class UGameDS_EquipmentWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	UGameDS_EquipmentWidget(const FObjectInitializer &ObjectInitializer);
	
	virtual void OnAppeared() override;
	virtual void OnDisappeared() override;

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeInfoType(EGameDS_ItemType InItemType);

	void UpdateStat(const UGameDS_StatController* InStatController);
	void UpdateInventory(UGameDS_InventoryController* InInventoryController);

private:
	void ChangeTileViewSetting(EGameDS_TileViewState InTileViewState);
	void InsertItemToTileViewObject(UGameDS_ItemInfoObject* Object, UGameDS_InventoryItemInfo* InventoryItemInfo);
	void InsertItemToTileViewObject(UGameDS_ItemInfoObject* Object, EGameDS_EquipSlotType InEquipSlot);
	
	void UpdateImageWithItemInfo(TArray<UGameDS_ItemInfoWidget*>& Target, TArray<UGameDS_InventoryItemInfo*> ItemInfoList);
	void UpdateImageWithItemInfo(TArray<UGameDS_ItemInfoWidget*>& Target, TArray<UGameDS_WeaponItemInfo*> ItemInfoList, bool bLWeapon);
	void UpdateImageWithItemInfo(TArray<UGameDS_ItemInfoWidget*>& Target, TArray<UGameDS_PotionItemInfo*> ItemInfoList);
	void OnClickInventoryItem(UGameDS_ItemInfoWidget* InItemInfoWidget);
	
	UFUNCTION()
	void OnClickExitInventoryTileView();
	
	// Equipped Item
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedRWeapon1;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedRWeapon2;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedRWeapon3;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedRWeapon4;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedRWeapon5;

	TArray<UGameDS_ItemInfoWidget*> EquippedRWeaponList;
	
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedLWeapon1;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedLWeapon2;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedLWeapon3;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedLWeapon4;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedLWeapon5;

	TArray<UGameDS_ItemInfoWidget*> EquippedLWeaponList;

	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedPotion1;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* EquippedPotion2;

	TArray<UGameDS_ItemInfoWidget*> EquippedPotionList;

	// 기타 아이템 (Default), 인벤 아이템 (장비 슬롯 누르면 열림)
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* LeftInfoSwitcher;

	// Owned Item
	UPROPERTY(meta=(BindWidget))
	UTileView* MiscellaneousTileView;
	
	// 한 줄당 나와야 하는 슬롯이 같아야 하므로 특정 타입의 최대 아이템 개수 * typeNum(5)를 해야 함
	// 물론 최소는 5 * typeNum(5)
	UPROPERTY(meta=(BindWidget))
	UTileView* InventoryTileView;

	UPROPERTY(meta=(BindWidget))
	UButton* ExitInventoryTileViewButton;
	//

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemType;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UTextBlock* WeaponType;
	UPROPERTY(meta=(BindWidget))
	UGameDS_ItemInfoWidget* SelectItem;

	// Attack Power
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Physical;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Magic;

	// Attribute Required
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RequiredVitality;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RequiredEndurance;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RequiredStrength;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RequiredIntelligence;

	// Guarded Damage Negation (이건 방어구 아이템 추가되기 전에는 사용할 일 없을듯, 일단 스킵)

	// Potion Effect
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* PotionHeld;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* HealValue;

	// Description
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Description;
	
	// Character Status
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Level;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* SoulHeld;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* HP;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* FP;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* SP;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Vitality;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Endurance;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Strength;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Intelligence;

	UPROPERTY()
	UGameDS_InventoryItemInfo* LastSelectItem;

	EGameDS_EquipSlotType LastSelectEquipSlot;

	EGameDS_TileViewState TileViewState = EGameDS_TileViewState::DefaultView;
};