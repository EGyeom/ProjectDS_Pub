// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "GameDS_EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ProjectDS/Unit/Enemy/GameDS_FirstBossAIController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectDS/Unit/Enemy/GameDS_BossAnimInstance.h"
#include "Components/StaticMeshComponent.h"

AGameDS_Boss::AGameDS_Boss()
{
	PrimaryActorTick.bCanEverTick = true;
	DebugUnitInfoWidget->SetActive(false);
	HealthWidget->SetActive(false);
	Phase = 1;
	bPhaseEnd = false;
}

bool AGameDS_Boss::IsUnitType(EGameDS_UnitType InUnitType)
{
	if (InUnitType == EGameDS_UnitType::Boss)
		return true;
	
	return Super::IsUnitType(InUnitType);
}

void AGameDS_Boss::BeginPlay()
{
	Super::BeginPlay();

	if (AIController != nullptr)
		BossAIController = Cast<AGameDS_FirstBossAIController>(AIController);

	if (AnimInstance != nullptr)
		BossAnimInstance = Cast<UGameDS_BossAnimInstance>(AnimInstance);
}

void AGameDS_Boss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGameDS_Boss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->UpdateBossStat(GetStatController());
	}
}

void AGameDS_Boss::SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo, const FGameDS_HeroStatInfo& InHeroStatInfo)
{
	Super::SpawnInit(InSpawnOption, InUnitStatInfo, InHeroStatInfo);

	UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>();
	if (UnitManager == nullptr)
		return;

	AGameDS_Character* LocalCharacter = UnitManager->GetLocalCharacter();
	if (LocalCharacter == nullptr)
		return;

	TargetCharacter = LocalCharacter;
	bFindTarget = true;
	AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(TargetCharacter);
	
	BossAIController->ChangeBattleModeSetting(true, Hero);

	if (BossAIController->ResetTargetDelegate.IsBound() == false)
	{
		BossAIController->ResetTargetDelegate.BindLambda([&]
			{
				TargetCharacter = nullptr;
				bFindTarget = false;
				UpdateHealthWidgetVisible(false);
			});
	}

	UStaticMeshComponent* WeaponMesh = FindComponentByClass<UStaticMeshComponent>();
	Phase2Effect = Cast<UActorComponent>(WeaponMesh->GetChildComponent(0));

	if (Phase2Effect == nullptr)
		return;

	Phase2Effect->SetActive(false);

	SetPhaseInfo(1);
}


void AGameDS_Boss::OnDamaged(const FGameDS_DamageInfo& InDamageInfo)
{
	if (IsDied() == true)
		return;

	if (bPhaseEnd == true)
		return;

	UGameDS_StatController* BossStatController = GetStatController();
	if (BossStatController == nullptr)
		return;

	TArray<EGameDS_StatResult> StatResultList;
	BossStatController->OnDamaged(InDamageInfo.DamageValue, StatResultList);

	if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->UpdateBossStat(BossStatController);
	}

	if (DamagedSound != nullptr)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DamagedSound, GetActorLocation());

	OnDamageWithBP(InDamageInfo.HitLocation, InDamageInfo.HitNormal);

	if (StatResultList.Find(EGameDS_StatResult::ZeroHealth) != INDEX_NONE)
	{
		SetPhaseInfo(0);

		if (Phase == 1)
		{
			bPhaseEnd = true;
			EndPhase(Phase);
			return;
		}

		SetDie(true);

		if (OnDeadDelegate.IsBound())
			OnDeadDelegate.Broadcast();

		if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
		{
			UnitManager->DespawnCharacter(GetUnitID());
		}
	}
}

void AGameDS_Boss::OnReset()
{
	Super::OnReset();
	
	if(IsDied() == false)
	{
		Destroy();
	}

}

void AGameDS_Boss::EndPhase(uint8 InPhase)
{
	if (AnimInstance == nullptr)
		return;

	AnimInstance->PlayMontage(EGameDS_UnitAnimType::Die);
}

void AGameDS_Boss::ChangePhase(uint8 InPhase)
{
	if (InPhase == 2)
	{
		Phase = InPhase;
		BossAIController->ChangeBattleModeType(EGameDS_EnemyBattleModeType::BossSecondPhaseMode);

		TArray<EGameDS_StatResult> StatResultList;
		GetStatController()->OnHealed(GetStatController()->GetStatInfoConfig().Health, StatResultList);

		if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
		{
			UIManager->OpenInfoUI(EGameDS_InfoUIType::BossUI);
		}
		SetPhaseInfo(2);

		if (Phase2Effect == nullptr)
			return;

		Phase2Effect->SetActive(true);
		bPhaseEnd = false;
	}
}

void AGameDS_Boss::UpdateHealthWidgetVisibleTime(float DeltaTime)
{
	//skip
}
