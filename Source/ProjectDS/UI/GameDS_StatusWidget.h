#pragma once
#include "GameDS_UserWidget.h"
#include "GameDS_StatusWidget.generated.h"

class UGameDS_InventoryController;
class UGameDS_StatInfoWidget;
class UGameDS_StatController;

UCLASS()
class UGameDS_StatusWidget : public UGameDS_UserWidget
{
	GENERATED_BODY()
public:
	virtual void OnAppeared() override;
	virtual void OnDisappeared() override;

	void UpdateStat(UGameDS_StatController* InStatController);
	void UpdateInventory(UGameDS_InventoryController* InInventoryController);

private:
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Level;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* SoulHeld;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* SoulNeeded;

	// Attribute Points
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Vitality;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Endurance;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Strength;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Intelligence;

	// Base Stats
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* HP;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* FP;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* SP;

	// Attack Power
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RWeapon1;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RWeapon2;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* RWeapon3;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* LWeapon1;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* LWeapon2;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* LWeapon3;

	// Guarded Damage Negation
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Physical;
	UPROPERTY(meta=(BindWidget))
	UGameDS_StatInfoWidget* Magic;
};
