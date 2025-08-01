#pragma once
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "GameDS_InventoryController.generated.h"

struct FGameDS_ItemDataTable;
class AGameDS_HeroCharacter;
enum class EGameDS_WeaponType : uint8;
enum class EGameDS_ItemType : uint8;

UENUM()
enum class EGameDS_EquipSlotType : uint8
{
	None,
	LWeapon1,
	LWeapon2,
	LWeapon3,
	LWeapon4,
	LWeapon5,
	RWeapon1,
	RWeapon2,
	RWeapon3,
	RWeapon4,
	RWeapon5,
	Potion1,
	Potion2
};

enum class EGameDS_ItemInnerType : uint8
{
	None,
	LWeapon,
	RWeapon,
	Potion
};

UCLASS()
class UGameDS_InventoryItemInfo : public UObject
{
	GENERATED_BODY()
public:
	int32 DataID = INDEX_NONE;
	int32 SerialID = INDEX_NONE;
	
	EGameDS_ItemType ItemType;
	EGameDS_EquipSlotType EquipSlotType;
	
	FString ItemName;
	int32 ItemCount = 0;
	int32 EnhancementLevel = 1;
	bool bEquipped = false;
	bool bMainItem = false;
};

UCLASS()
class UGameDS_WeaponItemInfo : public UGameDS_InventoryItemInfo
{
	GENERATED_BODY()
public:
	EGameDS_WeaponType WeaponType;

	float PhysicalDamage = 0.0f;
	float MagicDamage = 0.0f;

	int32 RequiredVitality = 0.0f;
	int32 RequiredEndurance = 0.0f;
	int32 RequiredStrength = 0.0f;
	int32 RequiredIntelligence= 0.0f;
};

UCLASS()
class UGameDS_PotionItemInfo : public UGameDS_InventoryItemInfo
{
	GENERATED_BODY()
public:
	float HealValue = 0;
	float CoolTime = 0;
	int32 DrinkableCount = 0;
	int32 MaximumDrinkableCount = 0;
};

UCLASS()
class UGameDS_InventoryController : public UObject
{
	GENERATED_BODY()

public:
	void InitController(AGameDS_HeroCharacter* InOwner);

	void LoadItem(const FGameDS_HeroCreateInfo& InHeroCreateInfo);
	void SaveItem();

	int32 AddItem(int32 DataID);
	UGameDS_InventoryItemInfo* GetItemForSerialID(int32 InSerialID);
	bool EquipItem(int32 DataID, EGameDS_EquipSlotType EquipSlot, int32 SerialID = INDEX_NONE);
	UGameDS_InventoryItemInfo* GetMainItem(EGameDS_ItemType InItemType, bool bLHand = false);
	void UseMainPotion();
	void SwapItem(EGameDS_ItemType ItemType, bool bLeftHand = false);

	EGameDS_ItemInnerType GetSlotTypeToItemInnerType(EGameDS_EquipSlotType InSlotType) const;
	
	UGameDS_WeaponItemInfo* GetMainWeapon(bool bLeftHandWeapon);
	UGameDS_PotionItemInfo* GetMainPotion();
	UGameDS_WeaponItemInfo* GetEquippedWeapon(EGameDS_EquipSlotType InEquipSlot);
	UGameDS_PotionItemInfo* GetEquippedPotion(EGameDS_EquipSlotType InEquipSlot);
	float GetEquippedWeaponDamage(EGameDS_EquipSlotType InEquipSlot);
	TArray<int32> GetEquippedWeaponIDList() const;
	TArray<int32> GetEquippedWeaponIDList(bool bLeftHandWeapon);
	TArray<int32> GetEquippedPotionIDList() const;
	TArray<int32> GetAllWeaponIDList(EGameDS_WeaponType InWeaponType) const;
	TArray<int32> GetAllPotionIDList() const;
	TArray<int32> GetEquippedSerialIDList() const;
	
	TArray<UGameDS_WeaponItemInfo*> GetEquippedWeaponInfoList(bool bLeftHandWeapon);
	TArray<UGameDS_PotionItemInfo*> GetEquippedPotionInfoList();
	TArray<UGameDS_WeaponItemInfo*> GetAllWeaponInfoList(EGameDS_WeaponType InWeaponType);
	TArray<UGameDS_PotionItemInfo*> GetAllPotionInfoList();
	
	void ResetAllPotionCount();

	int32 GetHeldSoul() const { return HeldSoul; }
	int32 GetNeededSoul() const { return NeededSoul; }
	int32 GetPhysicalGuard() const { return PhysicalGuard; }
	int32 GetMagicGuard() const { return MagicGuard; }

	bool IsValidMainItem(EGameDS_ItemInnerType InItemInnerType) const;
	void ChangeInventoryCallBack();

private:
	bool IsValidSlotType(int32 InDataID, EGameDS_EquipSlotType InEquipSlot) const;
	bool CheckEquippedSlot(EGameDS_EquipSlotType InEquipSlot) const;

	FSimpleDelegate InventoryChangeDelegate;

	UPROPERTY()
	TArray<UGameDS_InventoryItemInfo*> ItemInfoList;
	UPROPERTY()
	TArray<UGameDS_WeaponItemInfo*> WeaponInfoList;
	UPROPERTY()
	TArray<UGameDS_PotionItemInfo*> PotionInfoList;
	
	int32 HeldSoul;
	int32 NeededSoul;

	int32 PhysicalGuard;
	int32 MagicGuard;

	EGameDS_EquipSlotType MainRWeaponSlot;
	EGameDS_EquipSlotType MainLWeaponSlot;
	EGameDS_EquipSlotType MainPotionSlot;

	UPROPERTY()
	AGameDS_HeroCharacter* Owner;

	UPROPERTY()
	TMap<EGameDS_EquipSlotType, UGameDS_InventoryItemInfo*> EquippedItemMap;
};
