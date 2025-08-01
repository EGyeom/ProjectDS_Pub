#include "GameDS_UnitManager.h"
#include "GameDS_CustomDatas.h"
#include "GameDS_DataManager.h"
#include "GameDS_SummonManager.h"
#include "GameDS_UIManager.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Unit/Summon/GameDS_Summon.h"
#include "ProjectDS/Util/GameDS_Util.h"
#include "GameDS_BossBattleSubSystem.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"
#include "ProjectDS/Unit/Enemy/GameDS_FirstBossAIController.h"

AGameDS_Character* UGameDS_UnitManager::GetCharacter(int32 UnitID)
{
	return GetCharacter<AGameDS_Character>(UnitID);
}

AGameDS_Character* UGameDS_UnitManager::GetLocalCharacter()
{
	return GetCharacter<AGameDS_Character>(LocalID);
}

void UGameDS_UnitManager::GetAllCharacter(EGameDS_UnitType InUnitType, TArray<AGameDS_Character*>& CharacterList)
{
	for (const auto& PairData : CharacterMap)
	{
		AGameDS_Character* Character = PairData.Value;
		if (Character == nullptr)
			continue;

		if (Character->IsUnitType(InUnitType))
			CharacterList.Add(Character);
	}
}

int32 UGameDS_UnitManager::GetLastInteractionID()
{
	const AGameDS_HeroCharacter* LocalCharacter = GetLocalCharacter<AGameDS_HeroCharacter>();
	if (LocalCharacter == nullptr)
		return 0;

	return LocalCharacter->GetLastInteractionID();
}

int32 UGameDS_UnitManager::SpawnCharacter(const FGameDS_UnitSpawnOption& InSpawnOption)
{
	if (UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance()))
	{
		const FGameDS_SpawnData* SpawnData = DataManager->GetSpawnData(InSpawnOption);
		if (SpawnData == nullptr)
			return INDEX_NONE;

		if (CharacterMap.Find(InSpawnOption.UnitID) != nullptr)
		{
			AGameDS_Character* Character = *CharacterMap.Find(InSpawnOption.UnitID);
			Character->OnReset();
			
			CharacterMap.Remove(InSpawnOption.UnitID);
			Character->Destroy();
		}
		
		AActor* Actor = GetWorld()->SpawnActor(SpawnData->CharacterBP, &InSpawnOption.Location, &InSpawnOption.Rotator);
		
		if (AGameDS_Character* Character = Cast<AGameDS_Character>(Actor))
		{
			bool bHero = false;
			switch (InSpawnOption.UnitType)
			{
				case EGameDS_UnitType::Hero:
					{
						LocalID = InSpawnOption.UnitID;
						bHero = true;
					}
					break;
				case EGameDS_UnitType::Boss:
					{
						BossID = InSpawnOption.UnitID;
					}
					break;
				default:
					break;
			}

			CharacterMap.Add(InSpawnOption.UnitID, Character);

			FGameDS_UnitStatInfo StatInfo;
			FGameDS_HeroStatInfo HeroStatInfo;
			
			if (bHero)
			{
				if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
					UIManager->SetVisibleHeroUI(EGameDS_HeroUIType::InGameUI, true);
				
				FGameDS_HeroCreateInfo HeroCreateInfo;
				DataManager->LoadHeroData(HeroCreateInfo);

				FGameDS_Util::ParsingUnitInfo(HeroCreateInfo, HeroStatInfo, StatInfo);
			}
			else
			{
				DataManager->GetEnemyStatConfigData(InSpawnOption.DataID, StatInfo);	
			}
			
			Character->SpawnInit(InSpawnOption, StatInfo, HeroStatInfo);

			if (AllSpawnDataList.Find(InSpawnOption.UnitID) == nullptr)
				AllSpawnDataList.Add(InSpawnOption.UnitID, InSpawnOption);

			return InSpawnOption.UnitID;
		}
	}
	
	return INDEX_NONE;
}

void UGameDS_UnitManager::DespawnCharacter(int32 UnitID)
{
	AGameDS_Character** CharacterPtr = CharacterMap.Find(UnitID);
	
	if (CharacterPtr == nullptr)
		return;
	
	AGameDS_Character* Character = *CharacterPtr;
	
	if (Character != nullptr)
	{
		CharacterMap.Remove(UnitID);
		if (UnitID == BossID)
		{
			Character->OnReset();
			BossID = INDEX_NONE;
		}
	}	
}

int32 UGameDS_UnitManager::SpawnSummonObject(const FGameDS_SummonUnitSpawnOption& InSummonSpawnOption)
{
	UGameDS_BattleSubSystem* BattleSubSystem = UGameInstance::GetSubsystem<UGameDS_BattleSubSystem>(GetGameInstance());
	if (BattleSubSystem == nullptr)
		return INDEX_NONE;
	
	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DataManager == nullptr)
		return INDEX_NONE;

	const FGameDS_SummonDataTable* SummonRowData = DataManager->GetDataInRow<FGameDS_SummonDataTable>(
		InSummonSpawnOption.SummonDataID);
	if (SummonRowData == nullptr)
		return INDEX_NONE;

	AGameDS_Character* OwnerCharacter = GetCharacter(InSummonSpawnOption.OwnerUnitID);
	if (OwnerCharacter == nullptr)
		return INDEX_NONE;

	AGameDS_Character* TargetCharacter = GetCharacter(InSummonSpawnOption.TargetUnitID);

	const FGameDS_DetailCollisionInfo DetailCollisionInfo = BattleSubSystem->MakeSkillCollision(SummonRowData->SkillInfoID, OwnerCharacter, true, true);
	
	AActor* SummonActor = GetWorld()->SpawnActor(SummonRowData->SummonBP, &InSummonSpawnOption.Location, &InSummonSpawnOption.Rotator);
	if (AGameDS_Summon* Summon = Cast<AGameDS_Summon>(SummonActor))
	{
		if (SummonRowData->SpawnType == EGameDS_SkillSpawnType::CasterForward)
		{
			
		}
		else
		{
			Summon->SetActorRotation(OwnerCharacter->GetActorRotation());
		}
		
		if (SummonRowData->SummonType == EGameDS_SummonType::Follower)
		{
			if (TargetCharacter != nullptr)
				Summon->SetActorRotation((TargetCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation()).ToOrientationRotator());
		}
		
		Summon->SummonCollisionCheckDelegate.BindUObject(BattleSubSystem, &UGameDS_BattleSubSystem::OnAttackCheck);
		Summon->Init(OwnerCharacter, TargetCharacter, DetailCollisionInfo, SummonRowData);

		if (UGameDS_SummonManager* SummonManager = FGameInstanceHolder::GetManager<UGameDS_SummonManager>())
			SummonManager->AddSummon(Summon);

		return InSummonSpawnOption.SummonUnitID;
	}

	return INDEX_NONE;
}

void UGameDS_UnitManager::RespawnAllUnit()
{
	for (auto PairData : AllSpawnDataList)
	{
		const int32 CurrentUnitID = PairData.Value.UnitID;
		if (CurrentUnitID == BossID)
		{
			AGameDS_Boss* Boss = Cast<AGameDS_Boss>(*CharacterMap.Find(CurrentUnitID));
			if (Boss == nullptr)
				return;

			AGameDS_FirstBossAIController* BossAIC = Cast<AGameDS_FirstBossAIController>(Boss->GetController());
			if (BossAIC == nullptr)
				return;

			BossAIC->ResetBoss(false);
			continue;
		}

		SpawnCharacter(PairData.Value);
	}
}

int32 UGameDS_UnitManager::GetSummonOwnerUnitID(int32 InSummonUnitID)
{
	const AGameDS_Summon* SummonUnit = *SummonMap.Find(InSummonUnitID);
	if (SummonUnit == nullptr)
		return INDEX_NONE;

	return	SummonUnit->GetSummonOwnerUnitID();
}

float UGameDS_UnitManager::GetDistBetweenUnit(int32 FromUnitID, int32 ToUnitID)
{
	const float ResultDistance = INDEX_NONE;

	const AGameDS_Character* FromUnit = GetCharacter(FromUnitID);
	const AGameDS_Character* ToUnit = GetCharacter(ToUnitID);

	if (FromUnit != nullptr && ToUnit != nullptr)
	{
		return FVector::Dist(FromUnit->GetActorLocation(), ToUnit->GetActorLocation());
	}
	
	// 두 유닛 중 하나라도 nullptr일 때
	return ResultDistance;
}

float UGameDS_UnitManager::GetDistFromUser(int32 ToUnitID)
{
	return GetDistBetweenUnit(LocalID, ToUnitID);
}
