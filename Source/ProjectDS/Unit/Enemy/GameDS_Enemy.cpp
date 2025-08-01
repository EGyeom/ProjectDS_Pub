// Fill out your copyright notice in the Description page of Project Settings.


#include "GameDS_Enemy.h"

#include "BrainComponent.h"
#include "GameDS_EnemyAIController.h"
#include "Components/WidgetComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/UI/GameDS_StatGaugeWidget.h"
#include "ProjectDS/UI/Debug/GameDS_UnitInfoWidget.h"
#include "ProjectDS/Unit/GameDS_UnitAnimInstance.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/UI/GameDS_LockOnWidget.h"
#include "ProjectDS/Util/GameDS_Util.h"

AGameDS_Enemy::AGameDS_Enemy()
{ 
	PrimaryActorTick.bCanEverTick = true;
	
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));

	HealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
	HealthWidget->SetupAttachment(GetRootComponent());

	DebugUnitInfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DebugUnitInfoWidget"));
	DebugUnitInfoWidget->SetupAttachment(HealthWidget);

	LockOnWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidget"));
	LockOnWidget->SetupAttachment(GetMesh());
}

void AGameDS_Enemy::BeginPlay()
{
	Super::BeginPlay();

	PawnSensing->OnSeePawn.AddUniqueDynamic(this, &AGameDS_Enemy::OnSeePawn);

	if (UAnimInstance* AnimClass = GetMesh()->GetAnimInstance())
	{
		if (UGameDS_UnitAnimInstance* UnitAnimClass = Cast<UGameDS_UnitAnimInstance>(AnimClass))
		{
			AnimInstance = UnitAnimClass;
			AnimInstance->ChangeBattleMode(bBattleMode);
		}
	}

	if (HealthWidget != nullptr)
	{
		HealthWidget->InitWidget();
		if (UUserWidget* Widget = HealthWidget->GetUserWidgetObject())
		{
			if (UGameDS_StatGaugeWidget* GaugeWidget = Cast<UGameDS_StatGaugeWidget>(Widget))
			{
				HealthGaugeWidget = GaugeWidget;
				UpdateHealthWidgetVisible(false);
			}
		}
	}

	if (DebugUnitInfoWidget != nullptr)
	{
		DebugUnitInfoWidget->InitWidget();
		if (UUserWidget* Widget = DebugUnitInfoWidget->GetUserWidgetObject())
		{
			if (UGameDS_UnitInfoWidget* InfoWidget = Cast<UGameDS_UnitInfoWidget>(Widget))
			{
				UnitInfoWidget = InfoWidget;
				UnitInfoWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	if (BehaviorTree != nullptr)
	{
		AIController = Cast<AGameDS_EnemyAIController>(GetController());

		if (AIController != nullptr)
		{
			AIController->Init(this, AnimInstance);
			AIController->RunBehaviorTree(BehaviorTree);
		}
	}

	if (LockOnWidget != nullptr)
	{
		LockOnWidget->InitWidget();
		if (UUserWidget* Widget = LockOnWidget->GetUserWidgetObject())
		{
			LockOnWidget->SetWidgetClass(Widget->GetClass());
			if (UGameDS_LockOnWidget* TempLockOnWidget = Cast<UGameDS_LockOnWidget>(Widget))
				LockOnImageWidget = TempLockOnWidget;
		}
		LockOnWidget->SetWidgetSpace(EWidgetSpace::Screen);
		LockOnWidget->bHiddenInGame = true;
	}

	OnDeadDelegate.AddLambda([&]()
	{
		UpdateHealthWidgetVisible(false);
	});
}

void AGameDS_Enemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (PawnSensing && PawnSensing->OnSeePawn.IsBound())
		PawnSensing->OnSeePawn.RemoveDynamic(this, &AGameDS_Enemy::OnSeePawn);
}

void AGameDS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHealthWidgetVisible)
	{
		UpdateHealthWidgetVisibleTime(DeltaTime);
	}

	if (LockOnWidget->bHiddenInGame == false)
	{
		LockOnWidget->SetWorldLocation(GetMesh()->GetSocketLocation(GetLockOnSocketName()));
	}
}

void AGameDS_Enemy::SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo,
	const FGameDS_HeroStatInfo& InHeroStatInfo)
{
	Super::SpawnInit(InSpawnOption, InUnitStatInfo, InHeroStatInfo);

	if (AIController == nullptr)
		return;

	SpawnLocation = InSpawnOption.Location;
	SpawnRotator = FRotator(0.f, InSpawnOption.Rotator.Yaw, 0.f);

	AIController->LastInit();

	bBattleMode = InSpawnOption.bBattleMode;
	
	if (InSpawnOption.bUsePatrol)
	{
		SplinePathPoint = InSpawnOption.SplinePathPoint;

		AIController->InitPatrolPath(SplinePathPoint);
	}
}

bool AGameDS_Enemy::IsUnitType(EGameDS_UnitType InUnitType)
{
	if (InUnitType == EGameDS_UnitType::Enemy)
		return true;
	
	return Super::IsUnitType(InUnitType);
}

void AGameDS_Enemy::OnDamaged(const FGameDS_DamageInfo& InDamageInfo)
{
	Super::OnDamaged(InDamageInfo);

	if (GetUnitType() != EGameDS_UnitType::Boss)
	{
		if (bHealthWidgetVisible == false)
		{
			UpdateHealthWidgetVisible(true);
			bHealthWidgetVisible = true;
		}
		
		HealthWidgetVisibleTime = ENEMY_HEALTH_VISIBLE_TIME;
	}

	if (bFindTarget == false)
	{
		if (UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>())
		{
			int32 AttackedUnitID = INDEX_NONE;
			
			if (InDamageInfo.AttackedSummonUnitID != INDEX_NONE)
			{
				AttackedUnitID = UnitManager->GetSummonOwnerUnitID(InDamageInfo.AttackedSummonUnitID);
			}
			else if (InDamageInfo.AttackedUnitID != INDEX_NONE)
			{
				AttackedUnitID = InDamageInfo.AttackedUnitID;
			}

			if (IsDied() == false)
			{
				if (AGameDS_Character* AttackedCharacter = UnitManager->GetCharacter(AttackedUnitID))
					OnSeePawn(AttackedCharacter);
			}
		}
	}
}

void AGameDS_Enemy::UpdateHealthWidgetVisible(bool bInVisible)
{
	if (HealthGaugeWidget == nullptr)
		return;

	bHealthWidgetVisible = bInVisible;
	HealthGaugeWidget->SetVisibility(bInVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void AGameDS_Enemy::UpdateHealthWidgetVisibleTime(float DeltaTime)
{
	if (HealthWidgetVisibleTime > 0.0f)
	{
		HealthWidgetVisibleTime -= DeltaTime;
	}
	else
	{
		HealthWidgetVisibleTime = 0.0f;
		bHealthWidgetVisible = false;

		UpdateHealthWidgetVisible(false);
	}
}

void AGameDS_Enemy::OnReset()
{
	Super::OnReset();

	if (AIController)
	{
		AIController->UnPossess();
		AIController->DestroyWithDelay(5.0f);
		AIController = nullptr;
	}
}

void AGameDS_Enemy::OnSeePawn(APawn* InPawn)
{
	if (InPawn == nullptr)
		return;
	
	if (bFindTarget)
		return;

	if (AIController == nullptr)
		return;

	if (AnimInstance == nullptr)
		return;

	AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(InPawn);
	if (Hero == nullptr)
		return;
	
	if (Hero->IsInteraction())
		return;

	TargetCharacter = Hero;
	bFindTarget = true;
	
	AIController->ChangeBattleModeSetting(true, Hero);

	if (AIController->ResetTargetDelegate.IsBound() == false)
	{
		AIController->ResetTargetDelegate.BindLambda([&]
		{
			TargetCharacter = nullptr;
			bFindTarget = false;
			UpdateHealthWidgetVisible(false);
		});
	}
}

void AGameDS_Enemy::SetInitialPosition()
{
	SetActorLocation(SpawnLocation);
	SetActorRotation(SpawnRotator);
}

void AGameDS_Enemy::ToggleLockOnImage()
{
	LockOnWidget->bHiddenInGame = LockOnWidget->bHiddenInGame ? false : true;
}
