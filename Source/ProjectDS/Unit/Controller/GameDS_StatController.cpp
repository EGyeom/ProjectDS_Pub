#include "GameDS_StatController.h"

#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/ProjectDS.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"

UGameDS_StatController::UGameDS_StatController()
{
	StaminaRestoreType = EGameDS_StaminaRestoreType::Restore;
}

void UGameDS_StatController::Tick(float DeltaTime, TArray<EGameDS_StatResult>& StatResultList)
{
	if (OwnerCharacter->IsUnitType(EGameDS_UnitType::Hero) == false)
		return;
	
	UpdateStamina(DeltaTime);

	if (FMath::IsNearlyEqual(StatInfo.Stamina, StatInfoConfig.Stamina))
	{
		StatResultList.Add(EGameDS_StatResult::FullStamina);
	}
	else if (FMath::IsNearlyZero(StatInfo.Stamina))
	{
		StatResultList.Add(EGameDS_StatResult::ZeroStamina);
	}
}

void UGameDS_StatController::Init(const FGameDS_UnitStatInfo& LastStat)
{
	StatInfoConfig = LastStat;
	StatInfo = LastStat;
}

void UGameDS_StatController::Init(const FGameDS_HeroStatInfo& LastStat, AGameDS_Character* InCharacter)
{
	HeroStatInfo = LastStat;
	OwnerCharacter = InCharacter;

	UGameDS_DataManager* DataManager = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	const UGameDS_HeroStatConfigData* StatConfigData = DataManager->GetHeroStatConfigData();
	if (StatConfigData == nullptr)
		return;

	HeroStatConfigData = StatConfigData;
}

void UGameDS_StatController::LoadStat(const FGameDS_HeroCreateInfo& InHeroCreateInfo)
{
	const TArray<FString>& HeroStatList = InHeroCreateInfo.HeroStatList;

	if (HeroStatList.IsValidIndex(0))
		HeroStatInfo.Level = FCString::Atoi(*HeroStatList[0]);

	if (HeroStatList.IsValidIndex(1))
		HeroStatInfo.VitalityPoint = FCString::Atoi(*HeroStatList[1]);
		
	if (HeroStatList.IsValidIndex(2))
		HeroStatInfo.EndurancePoint = FCString::Atoi(*HeroStatList[2]);
	
	if (HeroStatList.IsValidIndex(3))
		HeroStatInfo.StrengthPoint = FCString::Atoi(*HeroStatList[3]);

	if (HeroStatList.IsValidIndex(4))
		HeroStatInfo.IntelligencePoint = FCString::Atoi(*HeroStatList[4]);

	GenderType = InHeroCreateInfo.GenderValue;
}

void UGameDS_StatController::SaveStat()
{
	UGameDS_DataManager* DataManger = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>();
	if (DataManger == nullptr)
		return;
	
	TArray<FString> HeroStatList;
	TArray<FString> UnitStatList;
	
	HeroStatList.Add(FString::FromInt(GetHeroStatInfo().Level));
	HeroStatList.Add(FString::FromInt(GetHeroStatInfo().VitalityPoint));
	HeroStatList.Add(FString::FromInt(GetHeroStatInfo().EndurancePoint));
	HeroStatList.Add(FString::FromInt(GetHeroStatInfo().StrengthPoint));
	HeroStatList.Add(FString::FromInt(GetHeroStatInfo().IntelligencePoint));
	DataManger->SaveData(STAT_INI_SECTION, HEROSTATINFO_INI_KEY, HeroStatList);

	UnitStatList.Add(FString::FromInt(GetStatInfoConfig().Health));
	UnitStatList.Add(FString::FromInt(GetStatInfoConfig().Focus));
	UnitStatList.Add(FString::FromInt(GetStatInfoConfig().Stamina));
	UnitStatList.Add(FString::FromInt(GetStatInfoConfig().PhysicalDefence));
	UnitStatList.Add(FString::FromInt(GetStatInfoConfig().MagicDefence));
	DataManger->SaveData(STAT_INI_SECTION, UNITSTATINFO_INI_KEY, UnitStatList);

	DataManger->SaveDataWithBool(STAT_INI_SECTION, GENDERINFO_INI_KEY, IsMale());
}

float UGameDS_StatController::GetStatValue(const EGameDS_StatType StatType) const
{
	switch (StatType)
	{
		case EGameDS_StatType::None:
			return INDEX_NONE;
		case EGameDS_StatType::Health:
			return StatInfo.Health;
		case EGameDS_StatType::Stamina:
			return StatInfo.Stamina;
		case EGameDS_StatType::MaximumHealth:
			return StatInfoConfig.Health;
		case EGameDS_StatType::MaximumStamina:
			return StatInfoConfig.Stamina;
		default: ;
	}
	return INDEX_NONE;
}

float UGameDS_StatController::GetHealthPercentage() const
{
	return StatInfo.Health / StatInfoConfig.Health;
}

float UGameDS_StatController::GetStaminaPercentage() const
{
    return StatInfo.Stamina / StatInfoConfig.Stamina;
}

void UGameDS_StatController::OnDamaged(float InDamageValue, TArray<EGameDS_StatResult>& StatResultList)
{
	StatInfo.Health -= InDamageValue;
	
	if (StatInfo.Health <= 0.0f)
	{
		StatInfo.Health = 0.0f;
		StatResultList.Add(EGameDS_StatResult::ZeroHealth);
	}
}

void UGameDS_StatController::OnHealed(float InHealValue, TArray<EGameDS_StatResult>& StatResultList)
{
	StatInfo.Health += InHealValue;

	if (StatInfo.Health >= StatInfoConfig.Health)
	{
		StatInfo.Health = StatInfoConfig.Health;
		StatResultList.Add(EGameDS_StatResult::FullHealth);
	}
}

void UGameDS_StatController::SetAction(EGameDS_HeroActionState InState)
{
	HeroActionState = InState;
}

bool UGameDS_StatController::OnUseStamina(float InRequiredStamina, TArray<EGameDS_StatResult>& StatResultList)
{
	if (StatInfo.Stamina < InRequiredStamina)
		return false;

	StatInfo.Stamina -= InRequiredStamina;
	if (StatInfo.Stamina <= 0.0f)
	{
		StatResultList.Add(EGameDS_StatResult::ZeroStamina);
	}

	return true;
}

bool UGameDS_StatController::OnUseStamina(EGameDS_HeroActionState InState, TArray<EGameDS_StatResult>& StatResultList)
{
	if (StaminaRestoreType == EGameDS_StaminaRestoreType::Exhaustion)
		return false;
	
	if (OwnerCharacter == nullptr)
		return false;

	if (OwnerCharacter->IsUnitType(EGameDS_UnitType::Hero) == false)
		return false;

	if (HeroStatConfigData == nullptr)
		return false;

	const float* RequiredStaminaValue = HeroStatConfigData->RequiredActionStamina.Find(InState);
	if (RequiredStaminaValue == nullptr)
		return false;

	if (*RequiredStaminaValue > 0.0f)
	{
		if (StatInfo.Stamina > *RequiredStaminaValue)
		{
			StaminaRestoreType = EGameDS_StaminaRestoreType::Delay;
			RestoredStaminaLockRemainTime = HeroStatConfigData->RestoredStaminaLockDuration;
		}
	}

	if (StatInfo.Stamina < *RequiredStaminaValue)
	{
		StaminaRestoreType = EGameDS_StaminaRestoreType::Exhaustion;
		ExhaustionRemainTime = HeroStatConfigData->ExhaustionDuration;
	}

	StatInfo.Stamina -= *RequiredStaminaValue;
	if (StatInfo.Stamina <= 0.0f)
	{
		StatInfo.Stamina = 0.0f;
		StatResultList.Add(EGameDS_StatResult::ZeroStamina);
	}

	HeroActionState = InState;

	return true;
}

void UGameDS_StatController::OnResetStatCondition()
{
	StatInfo.Health = StatInfoConfig.Health;
	StatInfo.Stamina = StatInfoConfig.Stamina;
}

bool UGameDS_StatController::IsMale() const
{
	return GenderType == EGameDS_GenderType::Male;
}

void UGameDS_StatController::UpdateStamina(float DeltaTime)
{
	switch (StaminaRestoreType)
	{
	case EGameDS_StaminaRestoreType::None:
	case EGameDS_StaminaRestoreType::Restore:
		{
			RestoreStamina(DeltaTime, 2.0f);
		}
		break;
	case EGameDS_StaminaRestoreType::Delay:
		{
			if (HeroActionState == EGameDS_HeroActionState::Run)
			{
				RestoreStamina(DeltaTime, -2.0f);
			}
			else if (HeroActionState == EGameDS_HeroActionState::StrongAttack)
			{
				RestoreStamina(DeltaTime, -2.0f);
			}
			else
			{
				RestoredStaminaLockRemainTime -= DeltaTime;
				
				if (RestoredStaminaLockRemainTime <= 0.0f)
				{
					RestoredStaminaLockRemainTime = 0.0f;
					StaminaRestoreType = EGameDS_StaminaRestoreType::Restore;
				}
			}
		}
		break;
	case EGameDS_StaminaRestoreType::Exhaustion:
		{
			ExhaustionRemainTime -= DeltaTime;

			if (ExhaustionRemainTime <= 0.0f)
			{
				ExhaustionRemainTime = 0.0f;

				StaminaRestoreType = EGameDS_StaminaRestoreType::None;

				RestoreStamina(DeltaTime, 2.0f);
				
				if (AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(OwnerCharacter))
					Hero->OnEndExhaustion();
			}
		}
		break;
	default: ;
	}

	if (AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(OwnerCharacter))
		Hero->OnUpdateUI();
}

void UGameDS_StatController::RestoreStamina(float DeltaTime, float Speed/*= 1.0f*/)
{
	StatInfo.Stamina += StatInfoConfig.Stamina * (0.1f * Speed) * DeltaTime;
			
	if (StatInfo.Stamina >= StatInfoConfig.Stamina)
	{
		StatInfo.Stamina = StatInfoConfig.Stamina;
		StaminaRestoreType = EGameDS_StaminaRestoreType::Restore;
	}
	else if (StatInfo.Stamina <= 0.0f)
	{
		if (HeroActionState == EGameDS_HeroActionState::StrongAttack)
		{
			if (AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(OwnerCharacter))
				Hero->OnStrongAttackReleased();
		}

		StatInfo.Stamina = 0.0f;
		ExhaustionRemainTime = HeroStatConfigData->ExhaustionDuration;
		StaminaRestoreType = EGameDS_StaminaRestoreType::Exhaustion;
	}
}
