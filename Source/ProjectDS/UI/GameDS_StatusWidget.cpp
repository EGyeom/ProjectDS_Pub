#include "GameDS_StatusWidget.h"

#include "GameDS_StatInfoWidget.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/Unit/Controller/GameDS_InventoryController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"

void UGameDS_StatusWidget::OnAppeared()
{
	Super::OnAppeared();
}

void UGameDS_StatusWidget::OnDisappeared()
{
	Super::OnDisappeared();

	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
		UIManager->SetInputMode(EGameDS_InputMode::GameOnly);
}

void UGameDS_StatusWidget::UpdateStat(UGameDS_StatController* InStatController)
{
	if (InStatController == nullptr)
		return;

	const FGameDS_HeroStatInfo& HeroStatInfo = InStatController->GetHeroStatInfo();
	const FGameDS_UnitStatInfo& StatInfoConfig = InStatController->GetStatInfoConfig();
	const FGameDS_UnitStatInfo& StatInfo = InStatController->GetStatInfo();

	Level->UpdateValue(0, HeroStatInfo.Level);

	// Attribute Points
	Vitality->UpdateValue(0, HeroStatInfo.VitalityPoint);
	Endurance->UpdateValue(0, HeroStatInfo.EndurancePoint);
	Strength->UpdateValue(0, HeroStatInfo.StrengthPoint);
	Intelligence->UpdateValue(0, HeroStatInfo.IntelligencePoint);

	// Base Stats
	HP->UpdateValue(StatInfo.Health, StatInfoConfig.Health);
	FP->UpdateValue(StatInfo.Focus, StatInfoConfig.Focus);
	SP->UpdateValue(StatInfo.Stamina, StatInfoConfig.Stamina);
}

void UGameDS_StatusWidget::UpdateInventory(UGameDS_InventoryController* InInventoryController)
{
	if (InInventoryController == nullptr)
		return;

	const int32 HeldSoul = InInventoryController->GetHeldSoul();
	const int32 NeededSoul = InInventoryController->GetNeededSoul();

	SoulHeld->UpdateValue(0, HeldSoul);
	SoulNeeded->UpdateValue(0, NeededSoul);

	// Attack Power
	LWeapon1->UpdateValue(0, InInventoryController->GetEquippedWeaponDamage(EGameDS_EquipSlotType::LWeapon1));
	LWeapon2->UpdateValue(0, InInventoryController->GetEquippedWeaponDamage(EGameDS_EquipSlotType::LWeapon2));
	LWeapon3->UpdateValue(0, InInventoryController->GetEquippedWeaponDamage(EGameDS_EquipSlotType::LWeapon3));
	RWeapon1->UpdateValue(0, InInventoryController->GetEquippedWeaponDamage(EGameDS_EquipSlotType::RWeapon1));
	RWeapon2->UpdateValue(0, InInventoryController->GetEquippedWeaponDamage(EGameDS_EquipSlotType::RWeapon2));
	RWeapon3->UpdateValue(0, InInventoryController->GetEquippedWeaponDamage(EGameDS_EquipSlotType::RWeapon3));

	// Guarded Damage Negation
	// 유닛 일반 스텟 + 방어력
	if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
	{
		const AGameDS_Character* LocalCharacter = UnitManager->GetLocalCharacter();

		if (LocalCharacter != nullptr && LocalCharacter->GetStatController())
		{
			const float TotalPhysicalGuard = LocalCharacter->GetStatController()->GetStatInfo().PhysicalDefence + InInventoryController->GetPhysicalGuard();
			const float TotalMagicGuard = LocalCharacter->GetStatController()->GetStatInfo().MagicDefence + InInventoryController->GetPhysicalGuard();
			
			Physical->UpdateValue(0, TotalPhysicalGuard);
			Magic->UpdateValue(0, TotalMagicGuard);
		}
	}
}
