#include "GameDS_UIManager.h"

#include "GameDS_UnitManager.h"
#include "ProjectDS/UI/GameDS_EquipmentWidget.h"
#include "ProjectDS/UI/GameDS_InGameWidget.h"
#include "ProjectDS/UI/GameDS_LevelUpWidget.h"
#include "ProjectDS/UI/GameDS_MenuWidget.h"
#include "ProjectDS/UI/GameDS_SoulShelterWidget.h"
#include "ProjectDS/UI/GameDS_StatGaugeWidget.h"
#include "ProjectDS/UI/GameDS_StatusWidget.h"
#include "ProjectDS/UI/Debug/GameDS_UnitInfoWidget.h"
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "ProjectDS/Unit/Controller/GameDS_InventoryController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"

void UGameDS_UIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InputMode = EGameDS_InputMode::GameOnly;

	InGameWidget = InitWidget<UGameDS_InGameWidget>(InGameWidgetPath);
	EquipmentWidget = InitWidget<UGameDS_EquipmentWidget>(EquipmentWidgetPath);
	StatusWidget = InitWidget<UGameDS_StatusWidget>(StatusWidgetPath);
	LevelUpWidget = InitWidget<UGameDS_LevelUpWidget>(LevelUpWidgetPath);
	SoulShelterWidget = InitWidget<UGameDS_SoulShelterWidget>(SoulShelterWidgetPath);
	MenuWidget = InitWidget<UGameDS_MenuWidget>(MenuWidgetPath);
}

void UGameDS_UIManager::SetControl(AGameDS_HeroCharacter* InOwnerHero, APlayerController* InPlayerController)
{
	OwnerHero = InOwnerHero;
	PlayerController = InPlayerController;

	if (OwnerHero != nullptr)
	{
		OwnerHero->StatUpdateDelegate.AddUObject(this, &UGameDS_UIManager::UpdateStat);
		OwnerHero->InventoryUpdateDelegate.AddUObject(this, &UGameDS_UIManager::UpdateInventory);

		// InGameUI를 업데이트 하기 위함
		UpdateInventory();
	}
}

void UGameDS_UIManager::OpenInfoUI(EGameDS_InfoUIType InInfoUIType, float InDuration)
{
	switch (InInfoUIType)
	{
	case EGameDS_InfoUIType::None:
		break;
	case EGameDS_InfoUIType::Interaction:
		{
			if (InGameWidget == nullptr)
				OpenInGameUI();

			InGameWidget->SetInteractionUI(ESlateVisibility::Visible);
		}
		break;
	case EGameDS_InfoUIType::BossUI:
	{
		if (InGameWidget == nullptr)
			OpenInGameUI();

		InGameWidget->SetActiveBossInfoUI(true);
	}
	break;
	default: ;
	}
}

void UGameDS_UIManager::RemoveInfoUI(EGameDS_InfoUIType InInfoUIType)
{
	switch (InInfoUIType)
	{
	case EGameDS_InfoUIType::None:
		break;
	case EGameDS_InfoUIType::Interaction:
		{
			if (InGameWidget != nullptr)
				InGameWidget->SetInteractionUI(ESlateVisibility::Hidden);
		}
		break;
	case EGameDS_InfoUIType::BossUI:
		{
			if (InGameWidget == nullptr)
				OpenInGameUI();
			
			InGameWidget->SetActiveBossInfoUI(false);
		}
	default: ;
	}
}

void UGameDS_UIManager::SetVisibleHeroUI(EGameDS_HeroUIType InHeroUIType, bool bVisible)
{
	const ESlateVisibility Visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	UGameDS_UserWidget* InUserWidget = nullptr;
	
	switch (InHeroUIType)
	{
	case EGameDS_HeroUIType::InGameUI:
		{
			if (InGameWidget == nullptr)
				OpenInGameUI();

			if (InGameWidget)
				InUserWidget = InGameWidget;
		}
		break;
	case EGameDS_HeroUIType::Status:
		{
			if (StatusWidget == nullptr)
				OpenHeroStatusUI();

			if (StatusWidget)
				InUserWidget = StatusWidget;
		}
		break;
	case EGameDS_HeroUIType::Equipment:
		{
			if (EquipmentWidget == nullptr)
				OpenHeroEquipmentUI();

			if (EquipmentWidget)
				InUserWidget = EquipmentWidget;
		}
		break;
	case EGameDS_HeroUIType::LevelUp:
		{
			if (LevelUpWidget == nullptr)
				OpenHeroLevelUpUI();

			if (LevelUpWidget)
				InUserWidget = LevelUpWidget;
		}
		break;
	case EGameDS_HeroUIType::SoulShelter:
		{
			if (SoulShelterWidget == nullptr)
				OpenSoulShelterUI();

			if (SoulShelterWidget)
				InUserWidget = SoulShelterWidget;
		}
		break;
	case EGameDS_HeroUIType::Menu:
		{
			if (MenuWidget == nullptr)
				OpenMenu();

			if (MenuWidget)
				InUserWidget = MenuWidget;
		}
		break;
	default: ;
	}

	if (InUserWidget != nullptr)
	{
		if (InUserWidget->IsInViewport() == false)
			InUserWidget->AddToViewport();
		
		InUserWidget->SetVisibility(Visibility);
		
		if (Visibility == ESlateVisibility::Visible)
		{
			if (InUserWidget->GetWidgetType() == EGameDS_WidgetType::MainUI)
				ActiveMainWidgetCount++;
		
			ActiveTypes.AddUnique(InHeroUIType);
		}
		else
		{
			if (InUserWidget->GetWidgetType() == EGameDS_WidgetType::MainUI)
				ActiveMainWidgetCount--;
			
			if (ActiveTypes.Find(InHeroUIType))
				ActiveTypes.Remove(InHeroUIType);
		}
	}

	if (ActiveMainWidgetCount > 0)
	{
		SetInputMode(EGameDS_InputMode::UIOnly);
	}
	else
	{
		SetInputMode(EGameDS_InputMode::GameOnly);
	}
}

bool UGameDS_UIManager::IsVisibleHeroUI(EGameDS_HeroUIType InHeroUIType) const
{
	switch (InHeroUIType)
	{
	case EGameDS_HeroUIType::InGameUI:
		{
			if (InGameWidget == nullptr)
				return false;

			return InGameWidget->IsVisible();
		}
	case EGameDS_HeroUIType::Status:
		{
			if (StatusWidget == nullptr)
				return false;
		
			return StatusWidget->IsVisible();
		}
	case EGameDS_HeroUIType::Equipment:
		{
			if (EquipmentWidget == nullptr)
				return false;
		
			return EquipmentWidget->IsVisible();
		}
	case EGameDS_HeroUIType::LevelUp:
		{
			if (LevelUpWidget == nullptr)
				return false;
		
			return LevelUpWidget->IsVisible();
		}
	case EGameDS_HeroUIType::SoulShelter:
		{
			if (SoulShelterWidget == nullptr)
				return false;

			return SoulShelterWidget->IsVisible();
		}
	case EGameDS_HeroUIType::Menu:
		{
			if (MenuWidget == nullptr)
				return false;

			return MenuWidget->IsVisible();
		}
	default: ;
	}

	return false;
}

void UGameDS_UIManager::UpdateUI(int32 UnitID, EGameDS_UnitType InUnitType, const UGameDS_StatController& InUnitStatController)
{
	if (InUnitType == EGameDS_UnitType::Hero)
	{
		UpdateStat();
	}
	else
	{
		// Health Bar Update
		if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
		{
			const AGameDS_Enemy* Enemy = UnitManager->GetCharacter<AGameDS_Enemy>(UnitID);
			if (Enemy == nullptr)
				return;

			if (UGameDS_StatGaugeWidget* GaugeWidget = Enemy->GetHealthGaugeWidget())
				GaugeWidget->UpdateGauge(InUnitStatController.GetHealthPercentage());

			if (UGameDS_UnitInfoWidget* UnitInfoWidget = Enemy->GetUnitInfoWidget())
				UnitInfoWidget->UpdateInfo(InUnitStatController.GetStatValue(EGameDS_StatType::Health));
		}
	}
}

void UGameDS_UIManager::SetInputMode(EGameDS_InputMode InInputMode)
{
	if (InputMode == InInputMode)
		return;

	InputMode = InInputMode;
	
	switch (InputMode)
	{
		case EGameDS_InputMode::GameOnly:
			PlayerController->SetShowMouseCursor(false);
			PlayerController->SetInputMode(FInputModeGameOnly());
			break;
		case EGameDS_InputMode::UIOnly:
			PlayerController->SetShowMouseCursor(true);
			PlayerController->SetInputMode(FInputModeUIOnly());
			break;
		case EGameDS_InputMode::GameAndUI:
			PlayerController->SetShowMouseCursor(true);
			PlayerController->SetInputMode(FInputModeGameAndUI());
			break;
		default: ;
	}
}

void UGameDS_UIManager::UpdateBossStat(UGameDS_StatController* InUnitStatController)
{
	if (InUnitStatController == nullptr)
		return;

	if (IsValidWidget(InGameWidget))
	{
		UGameDS_StatGaugeWidget* BossHPGauge = InGameWidget->GetBossHPGaugeWidget();
		if (BossHPGauge != nullptr)
			BossHPGauge->UpdateGauge(InUnitStatController->GetHealthPercentage());
	}
}

void UGameDS_UIManager::UpdateStat(UGameDS_StatController* InUnitStatController)
{
	if (OwnerHero == nullptr)
		return;
	
	if (InUnitStatController == nullptr)
		return;

	if (IsValidWidget(InGameWidget))
	{
		if (InGameWidget->GetHealthGaugeWidget() != nullptr)
			InGameWidget->GetHealthGaugeWidget()->UpdateGauge(InUnitStatController->GetHealthPercentage());

		if (InGameWidget->GetStaminaGaugeWidget() != nullptr)
			InGameWidget->GetStaminaGaugeWidget()->UpdateGauge(InUnitStatController->GetStaminaPercentage());
	}

	if (IsValidWidget(StatusWidget))
	{
		StatusWidget->UpdateStat(InUnitStatController);
	}
	if (IsValidWidget(EquipmentWidget))
	{
		EquipmentWidget->UpdateStat(InUnitStatController);
	}
}

void UGameDS_UIManager::UpdateStat()
{
	if (OwnerHero == nullptr)
		return;
	
	if (UGameDS_StatController* StatController = OwnerHero->GetStatController())
		UpdateStat(StatController);
}

void UGameDS_UIManager::UpdateInventory(UGameDS_InventoryController* InUnitInventoryController)
{
	if (OwnerHero == nullptr)
		return;
	
	if (InUnitInventoryController == nullptr)
		return;

	if (IsValidWidget(StatusWidget))
	{
		StatusWidget->UpdateInventory(InUnitInventoryController);
	}
	if (IsValidWidget(EquipmentWidget))
	{
		EquipmentWidget->UpdateInventory(InUnitInventoryController);
	}
	
	if (IsValidWidget(InGameWidget))
	{
		int32 LeftItemID = INDEX_NONE;
		int32 RightItemID = INDEX_NONE;
		int32 TopItemID = INDEX_NONE;
		int32 BottomItemID = INDEX_NONE;
		
		if (const UGameDS_WeaponItemInfo* LHand = InUnitInventoryController->GetMainWeapon(true))
			LeftItemID = LHand->DataID;
		
		if (const UGameDS_WeaponItemInfo* RHand = InUnitInventoryController->GetMainWeapon(false))
			RightItemID = RHand->DataID;
		
		if (const UGameDS_PotionItemInfo* Potion = InUnitInventoryController->GetMainPotion())
			BottomItemID = Potion->DataID;
		
		InGameWidget->UpdateItem(LeftItemID, RightItemID, TopItemID, BottomItemID);
	}
}

void UGameDS_UIManager::UpdateInventory()
{
	if (OwnerHero == nullptr)
		return;
	
	if (UGameDS_InventoryController* InventoryController = OwnerHero->GetInventoryController())
		UpdateInventory(InventoryController);
}

void UGameDS_UIManager::OpenHeroUI(EGameDS_HeroUIType InHeroUIType)
{
	switch (InHeroUIType)
	{
	case EGameDS_HeroUIType::InGameUI:
		OpenInGameUI();
		break;
	case EGameDS_HeroUIType::Menu:
		OpenMenu();
		break;
	case EGameDS_HeroUIType::Status:
		OpenHeroStatusUI();
		break;
	case EGameDS_HeroUIType::Equipment:
		OpenHeroEquipmentUI();
		break;
	case EGameDS_HeroUIType::LevelUp:
		OpenHeroLevelUpUI();
		break;
	case EGameDS_HeroUIType::SoulShelter:
		OpenSoulShelterUI();
		break;
	default: ;
	}
}

bool UGameDS_UIManager::IsValidWidget(UGameDS_UserWidget* InUserWidget)
{
	return InUserWidget != nullptr;// && InUserWidget->IsVisible();
}

void UGameDS_UIManager::OpenInGameUI()
{
	if (InGameWidget == nullptr)
		return;

	if (InGameWidget->IsInViewport() == false)
		InGameWidget->AddToViewport();

	InGameWidget->SetVisibility(ESlateVisibility::Visible);
}

void UGameDS_UIManager::OpenMenu()
{
	if (MenuWidget == nullptr)
		return;

	if (MenuWidget->IsInViewport() == false)
		MenuWidget->AddToViewport();

	MenuWidget->SetVisibility(ESlateVisibility::Visible);
}

void UGameDS_UIManager::OpenHeroStatusUI()
{
	if (StatusWidget == nullptr)
		return;

	if (PlayerController == nullptr)
		return;

	if (OwnerHero == nullptr)
		return;

	if (StatusWidget->IsInViewport() == false)
		StatusWidget->AddToViewport();

	StatusWidget->SetVisibility(ESlateVisibility::Visible);
	UpdateStat();
	UpdateInventory();
}

void UGameDS_UIManager::OpenHeroEquipmentUI()
{
	if (EquipmentWidget == nullptr)
		return;

	if (PlayerController == nullptr)
		return;

	if (EquipmentWidget->IsInViewport() == false)
		EquipmentWidget->AddToViewport();

	EquipmentWidget->SetVisibility(ESlateVisibility::Visible);

	UpdateStat();
	UpdateInventory();
}

void UGameDS_UIManager::OpenHeroLevelUpUI()
{
	if (LevelUpWidget == nullptr)
		return;

	if (PlayerController == nullptr)
		return;

	if (LevelUpWidget->IsInViewport() == false)
		LevelUpWidget->AddToViewport();

	LevelUpWidget->SetVisibility(ESlateVisibility::Visible);
}

void UGameDS_UIManager::OpenSoulShelterUI()
{
	if (SoulShelterWidget == nullptr)
		return;

	if (SoulShelterWidget->IsInViewport() == false)
		SoulShelterWidget->AddToViewport();

	SoulShelterWidget->SetVisibility(ESlateVisibility::Visible);
}
