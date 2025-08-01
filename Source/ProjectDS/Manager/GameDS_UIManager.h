#pragma once
#include "Blueprint/UserWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameDS_UIManager.generated.h"

class UGameDS_UserWidget;
class UGameDS_LevelUpWidget;
class UGameDS_StatusWidget;
class UGameDS_EquipmentWidget;
class UGameDS_InGameWidget;
class UGameDS_SoulShelterWidget;
class UGameDS_MenuWidget;
enum class EGameDS_UnitType : uint8;
class UGameDS_StatController;
class AGameDS_HeroCharacter;
class UGameDS_InventoryController;

UENUM(BlueprintType)
enum class EGameDS_HeroUIType : uint8
{
	None,
	InGameUI,
	Menu,
	Status,
	Equipment,
	LevelUp,
	SoulShelter
};

UENUM(BlueprintType)
enum class EGameDS_InfoUIType : uint8
{
	None,
	Interaction,
	BossUI
};

enum class EGameDS_InputMode
{
	None,
	GameOnly,
	UIOnly,
	GameAndUI
};

UCLASS(Config = Game)
class UGameDS_UIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void SetControl(AGameDS_HeroCharacter* InOwnerHero, APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable)
	void OpenInfoUI(EGameDS_InfoUIType InInfoUIType, float InDuration = -1.0f);
	UFUNCTION(BlueprintCallable)
	void RemoveInfoUI(EGameDS_InfoUIType InInfoUIType);

	void UpdateUI(int32 UnitID, EGameDS_UnitType InUnitType, const UGameDS_StatController& InUnitStatController);
	void SetInputMode(EGameDS_InputMode InInputMode);
	void UpdateBossStat(UGameDS_StatController* InUnitStatController);
	void UpdateStat(UGameDS_StatController* InUnitStatController);
	void UpdateStat();
	void UpdateInventory(UGameDS_InventoryController* InUnitInventoryController);
	void UpdateInventory();

	void OpenHeroUI(EGameDS_HeroUIType InHeroUIType);
	
	UFUNCTION(BlueprintCallable)
	void SetVisibleHeroUI(EGameDS_HeroUIType InHeroUIType, bool bVisible);
	UFUNCTION(BlueprintCallable)
	bool IsVisibleHeroUI(EGameDS_HeroUIType InHeroUIType) const;

private:
	template <typename WidgetClass>
	WidgetClass* InitWidget(FSoftClassPath SoftPath);
	
	bool IsValidWidget(UGameDS_UserWidget* InUserWidget);
	bool IsOpenedOtherUI() const;
	
	void OpenInGameUI();
	void OpenMenu();
	void OpenHeroStatusUI();
	void OpenHeroEquipmentUI();
	void OpenHeroLevelUpUI();
	void OpenSoulShelterUI();

	UPROPERTY(Config)
	FSoftClassPath InGameWidgetPath;
	UPROPERTY()
	UGameDS_InGameWidget* InGameWidget;
	
	UPROPERTY(Config)
	FSoftClassPath EquipmentWidgetPath;
	UPROPERTY()
	UGameDS_EquipmentWidget* EquipmentWidget;
	
	UPROPERTY(Config)
	FSoftClassPath StatusWidgetPath;
	UPROPERTY()
	UGameDS_StatusWidget* StatusWidget;

	UPROPERTY(Config)
	FSoftClassPath MenuWidgetPath;
	UPROPERTY()
	UGameDS_MenuWidget* MenuWidget;
	
	UPROPERTY(Config)
	FSoftClassPath LevelUpWidgetPath;
	UPROPERTY()
	UGameDS_LevelUpWidget* LevelUpWidget;

	UPROPERTY(Config)
	FSoftClassPath SoulShelterWidgetPath;
	UPROPERTY()
	UGameDS_SoulShelterWidget* SoulShelterWidget;

	TArray<EGameDS_HeroUIType> ActiveTypes;

	EGameDS_InputMode InputMode = EGameDS_InputMode::None;
	bool bRequestOpenInGameBeforeLoaded = false;

	UPROPERTY()
	APlayerController* PlayerController;
	UPROPERTY()
	AGameDS_HeroCharacter* OwnerHero;

	int32 ActiveMainWidgetCount = 0;
};

template <typename WidgetClass>
WidgetClass* UGameDS_UIManager::InitWidget(FSoftClassPath SoftPath)
{
	//WidgetClass = LoadObject<UBlueprintGeneratedClass>(nullptr, *AssetPath);
	if (UClass* BPClass = LoadObject<UBlueprintGeneratedClass>(nullptr, *SoftPath.GetAssetPathString()))
	{
		UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), BPClass);
		if (Widget == nullptr)
			return nullptr;

		return Cast<WidgetClass>(Widget);
	}
	
	return nullptr;
}
