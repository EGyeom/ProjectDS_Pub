#include "GameDS_Character.h"

#include "GameDS_UnitAnimInstance.h"
#include "Controller/GameDS_StatController.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "ProjectDS/Manager/GameDS_BattleSubSystem.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/UI/GameDS_LockOnWidget.h"
#include "Components/WidgetComponent.h"
#include "Controller/GameDS_CrowdControlController.h"
#include "Controller/GameDS_StatusEffectController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"

AGameDS_Character::AGameDS_Character()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGameDS_Character::BeginPlay()
{
	Super::BeginPlay();

	StatController = NewObject<UGameDS_StatController>();
	CrowdController = NewObject<UGameDS_CrowdControlController>();
	StatusEffectController = NewObject<UGameDS_StatusEffectController>();
	
	OnDamageDelegate.AddUObject(this, &AGameDS_Character::OnDamaged);
	OnDeadDelegate.AddUObject(this, &AGameDS_Character::OnDead);

	CrowdController->Init(this);
	StatusEffectController->Init(this);
}

void AGameDS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CrowdController->Tick(DeltaTime);
	StatusEffectController->Tick(DeltaTime);
}

void AGameDS_Character::SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo,
                                  const FGameDS_HeroStatInfo& InHeroStatInfo)
{
	UnitType = InSpawnOption.UnitType;
	UnitID = InSpawnOption.UnitID;
	DataID = InSpawnOption.DataID;
	
	if (StatController != nullptr)
	{
		StatController->Init(InUnitStatInfo);

		if (InSpawnOption.UnitType == EGameDS_UnitType::Hero)
			StatController->Init(InHeroStatInfo, this);
	}

	bDied = false;
}

bool AGameDS_Character::IsUnitType(EGameDS_UnitType InUnitType)
{
	return InUnitType == EGameDS_UnitType::None;
}

void AGameDS_Character::OnDamaged(const FGameDS_DamageInfo& InDamageInfo)
{
	if (bDied == true)
		return;

	if (StatController == nullptr)
		return;

	TArray<EGameDS_StatResult> StatResultList;
	StatController->OnDamaged(InDamageInfo.DamageValue, StatResultList);

	// ui update
	if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->UpdateUI(UnitID, UnitType, *StatController);
	}

	UpdateStat();

	if (DamagedSound != nullptr)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DamagedSound, GetActorLocation());

	OnDamageWithBP(InDamageInfo.HitLocation, InDamageInfo.HitNormal);

	if (StatResultList.Find(EGameDS_StatResult::ZeroHealth) != INDEX_NONE)
	{
		bDied = true;

		UCapsuleComponent* Capsule = GetCapsuleComponent();
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (OnDeadDelegate.IsBound())
			OnDeadDelegate.Broadcast();		

		if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
		{
			UnitManager->DespawnCharacter(UnitID);
		}
	}
}

void AGameDS_Character::OnReset()
{
}

void AGameDS_Character::OnDead()
{
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (UGameDS_UnitAnimInstance* UnitAnimInst = Cast<UGameDS_UnitAnimInstance>(AnimInst))
		{
			UnitAnimInst->StopAllMontages(0.5f);
			if (GetUnitType() == EGameDS_UnitType::Boss)
			{
				const AGameDS_Boss* Boss = Cast<AGameDS_Boss>(this);
				if (Boss == nullptr)
					return;
				
				if (Boss->GetPhase() == 2)
				{
					UnitAnimInst->PlayMontage(EGameDS_UnitAnimType::Die, 1);
					return;
				}
			}
			
			UnitAnimInst->PlayMontage(EGameDS_UnitAnimType::Die);
		}
	}
}

void AGameDS_Character::UpdateStat()
{
	if (StatController == nullptr)
		return;

	if (StatUpdateDelegate.IsBound() == false)
		return;

	StatUpdateDelegate.Broadcast(StatController);
}

void AGameDS_Character::PlayCCMotion(EGameDS_CrowdControlType InCCType)
{
	if (IsUnitType(EGameDS_UnitType::Hero) == false)
		return;

	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (UGameDS_UnitAnimInstance* UnitAnimInst = Cast<UGameDS_UnitAnimInstance>(AnimInst))
		{
			if (InCCType == EGameDS_CrowdControlType::NormalKnockBack || InCCType == EGameDS_CrowdControlType::HeavyKnockBack)
				UnitAnimInst->StopAllMontages(0.f);
			UnitAnimInst->PlayCCMotion(InCCType);
		}
	}
}