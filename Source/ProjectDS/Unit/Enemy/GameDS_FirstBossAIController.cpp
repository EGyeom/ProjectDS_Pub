// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Enemy/GameDS_FirstBossAIController.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"
#include "ProjectDS/Unit/Enemy/GameDS_EnemyAnimInstance.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"
#include "ProjectDS/Unit/Enemy/GameDS_BossAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectDS/Util/GameDS_Util.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameDS_FirstBossAIController.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"

AGameDS_FirstBossAIController::AGameDS_FirstBossAIController()
{
    TickCount = -0.2f;
    AttackDelay = -0.1f;
    bUsedSkill = false;
    bInteraction = false;
}

void AGameDS_FirstBossAIController::Init(AGameDS_Enemy* InEnemy, UGameDS_UnitAnimInstance* InAnimInstance)
{
    if (InEnemy == nullptr)
        return;
    if (InAnimInstance == nullptr)
        return;
    
    Super::Init(InEnemy, InAnimInstance);
   
    OwnerBoss = Cast<AGameDS_Boss>(InEnemy);
    if (OwnerBoss == nullptr)
        return;

    if (AnimInstance != nullptr)
    {
        OwnerAnimInstance = Cast<UGameDS_BossAnimInstance>(AnimInstance);

        AnimInstance->EndAttackDelegate.BindLambda([this]
            {
                UpdateState(EGameDS_EnemyAIMode::AttackRest);
                SetUseSkill(false);
            });
        
        AnimInstance->EndHitDelegate.BindLambda([this]
		{
            RestElapsedTime = 0.0f;
            RestDuration = 0.0f;
            bHit = false;
            AnimInstance->SetHit(false);

            if (AIMode != EGameDS_EnemyAIMode::PhaseEnd)
                UpdateState(EGameDS_EnemyAIMode::AttackSelection);
		});
    }

    bInteraction = false;
}

void AGameDS_FirstBossAIController::LastInit()
{
    Super::LastInit();

    GetBlackboardComponent()->SetValueAsBool(TEXT("bBoss"), StaticCast<bool>(true));
}

void AGameDS_FirstBossAIController::InitPatrolPath(const TArray<FVector>& SplinePath)
{
    Super::InitPatrolPath(SplinePath);
}

void AGameDS_FirstBossAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (OwnerEnemy == nullptr)
        return;

    if (Target != nullptr)
    {
        if (Target->IsInteraction())
        {
            if (bInteraction == false)
            {
                ResetBoss(true);
            }
        }
        else
        {
            bInteraction = false;
        }
    }
}

void AGameDS_FirstBossAIController::UpdateAction(float DeltaSeconds)
{
    Super::UpdateAction(DeltaSeconds);
}

void AGameDS_FirstBossAIController::UpdateRotateValue(float DeltaSeconds)
{
    Super::UpdateRotateValue(DeltaSeconds);
}

void AGameDS_FirstBossAIController::ChangeBattleModeSetting(bool bActive, AGameDS_HeroCharacter* InTarget)
{
    Super::ChangeBattleModeSetting(bActive, InTarget);
}

void AGameDS_FirstBossAIController::SetUseSkill(bool InUse)
{
    bUsedSkill = InUse;
    OwnerAnimInstance->SetRun(false);
    OwnerBoss->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->WalkSpeed;
}

void AGameDS_FirstBossAIController::ResetBoss(bool Interaction)
{
    if (OwnerBoss == nullptr)
        return;

    if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
    {
        if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(OwnerEnemy->GetGameInstance()))
        {
            UIManager->RemoveInfoUI(EGameDS_InfoUIType::BossUI);
        }
        
        if (OwnerBoss->OnDespawnDelegate.IsBound())
            OwnerBoss->OnDespawnDelegate.Broadcast();
        
        OwnerBoss->SetPhaseInfo(0);
        UnitManager->DespawnCharacter(OwnerEnemy->GetUnitID());

        bInteraction = Interaction;
    }
}
