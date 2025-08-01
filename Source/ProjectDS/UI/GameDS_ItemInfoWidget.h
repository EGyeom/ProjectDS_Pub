#pragma once
#include "GameDS_UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GameDS_ItemInfoWidget.generated.h"

enum class EGameDS_EquipSlotType : uint8;
enum class EGameDS_ItemType : uint8;
class UWidgetSwitcher;
class UImage;
class UButton;

DECLARE_DELEGATE_OneParam(FItemSelectDelegate, UGameDS_ItemInfoWidget* InItemInfo)

UCLASS()
class UGameDS_ItemInfoObject : public UObject
{
	GENERATED_BODY()
public:
	EGameDS_ItemType ItemType;
	int32 ItemID = INDEX_NONE;
	int32 ItemSerialID = INDEX_NONE;
	bool bEquipped = false;
	EGameDS_EquipSlotType EquipSlotType;

	FItemSelectDelegate EntrySelectDelegate;
};

UCLASS()
class UGameDS_ItemInfoWidget : public UGameDS_UserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void BeginDestroy() override;
	virtual void OnAppeared() override;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

	void UpdateItemInfo(EGameDS_ItemType InItemType, int32 InItemID, int32 InItemSerialID, EGameDS_EquipSlotType InEquipSlotType);
	void UpdateItemInfoOnlyImage(int32 InItemID);
	void UpdateItemEquipped(bool bInEquipped);
	int32 GetSerialID() const { return ItemSerialID; }
	UImage* GetItemImage() const { return ItemImage; }
	void SetBlockClickEvent(bool bEnable) { bBlockClick = bEnable; }
	void SetEnableSelectionButton(bool bEnable);
	EGameDS_EquipSlotType GetEquippedSlotType() const { return EquipSlotType; }

	UFUNCTION()
	void OnClickItem();

	FItemSelectDelegate ItemSelectDelegate;
	FItemSelectDelegate EntrySelectDelegate;

private:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UImage* ItemImage;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UImage* EquippedIcon;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UWidgetSwitcher* HintSwitcher;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget, AllowPrivateAccess))
	UButton* ItemSelectButton;

	EGameDS_ItemType ItemType;
	int32 ItemID = INDEX_NONE;
	int32 ItemSerialID = INDEX_NONE;
	bool bEquipped = false;
	EGameDS_EquipSlotType EquipSlotType;

	bool bBlockClick = false;
};