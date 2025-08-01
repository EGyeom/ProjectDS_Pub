#include "GameDS_EnemyAIController.h"

#include "GameDS_Enemy.h"
#include "GameDS_Boss.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Unit/GameDS_UnitAnimInstance.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"
#include "ProjectDS/Util/GameDS_Util.h"
#include "ProjectDS/Unit/Enemy/GameDS_BossAnimInstance.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"

void AGameDS_EnemyAIController::Init(AGameDS_Enemy* InEnemy, UGameDS_UnitAnimInstance* InAnimInstance)
{
	OwnerEnemy = InEnemy;
	AnimInstance = InAnimInstance;

	CachingSkillSetting();

	if (OwnerEnemy != nullptr)
	{
		OwnerEnemy->OnDamageDelegate.AddLambda([this](const FGameDS_DamageInfo& InDamageInfo)
		{
			PauseMove(GetCurrentMoveRequestID());
			UpdateState(EGameDS_EnemyAIMode::Hit);
		});
		OwnerEnemy->OnDeadDelegate.AddLambda([this]
		{
			PauseMove(GetCurrentMoveRequestID());
			UpdateState(EGameDS_EnemyAIMode::Die);
		});
	}
	if (AnimInstance != nullptr)
	{
		AnimInstance->EndAttackDelegate.BindLambda([this]
		{
			UpdateState(EGameDS_EnemyAIMode::AttackRest);
		});
		AnimInstance->EndHitDelegate.BindLambda([this]
		{
			RestElapsedTime = 0.0f;
			RestDuration = 0.0f;
			bHit = false;
			AnimInstance->SetHit(false);
			
			UpdateState(EGameDS_EnemyAIMode::AttackSelection);
		});
		AnimInstance->EndEvadeDelegate.BindLambda([this]
		{
			AnimInstance->SetEvade(false);
			UpdateState(EGameDS_EnemyAIMode::AttackSelection);
		});
		AnimInstance->EndModeChangeDelegate.BindLambda([this]
		{
			bPlayModeTransitionAnim = false;
			ResumeMove(GetCurrentMoveRequestID());
			UpdateState(EGameDS_EnemyAIMode::AttackSelection);
		});
	}
}

void AGameDS_EnemyAIController::LastInit()
{
	if (OwnerEnemy == nullptr)
		return;
	
	UGameDS_DataManager* DataManager = GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	const FGameDS_EnemyStatConfigDataTable* ConfigData = DataManager->GetDataInRow<FGameDS_EnemyStatConfigDataTable>(OwnerEnemy->GetDataID());
	if (ConfigData == nullptr)
		return;

	StatConfigData = ConfigData;
	bPatrolUnit = PatrolPoint.Num() > 1;

	DefaultBattleMode = StatConfigData->DefaultBattleMode;
	ChangeBattleModeSetting(false);

	GetBlackboardComponent()->SetValueAsBool(TEXT("bBoss"), StaticCast<bool>(false));
}

void AGameDS_EnemyAIController::InitPatrolPath(const TArray<FVector>& SplinePath)
{
	PatrolPoint = SplinePath;

	NextPatrolIndex = SplinePath.Num() >= 2 ? 1 : 0;

	UpdateState(EGameDS_EnemyAIMode::Patrol);
}

void AGameDS_EnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bActiveDestroy)
	{
		if (DestroyRemainTime > 0.0f)
		{
			DestroyRemainTime -= DeltaSeconds;
		}
		else
		{
			Destroy();
		}
		return;
	}

	if (Target != nullptr)
	{
		if (Target->IsInteraction())
		{
			if (OwnerEnemy->GetUnitType() == EGameDS_UnitType::Boss)
			{
				return;
			}

			if (AIMode == EGameDS_EnemyAIMode::FaceOff)
				SetFaceOffMode(false);

			ChangeBattleModeSetting(false);
			
			if (AnimInstance->Montage_IsPlaying(nullptr))
				AnimInstance->StopAllMontages(0.5f);
			
			UpdateState(EGameDS_EnemyAIMode::Idle);
			
			OwnerEnemy->SetInitialPosition();
			OwnerEnemy->GetStatController()->OnResetStatCondition();

		}
	}

	if (StatConfigData == nullptr)
		return;

	if (AIMode != EGameDS_EnemyAIMode::ModeTransition)
		ModeChangeDelayElapsedTime += DeltaSeconds;

	if (bSetTargetLocation == true)
		FollowTarget(DeltaSeconds);

	UpdateAction(DeltaSeconds);
	UpdateRotateValue(DeltaSeconds);
}

void AGameDS_EnemyAIController::UpdateAction(float DeltaSeconds)
{
	switch (AIMode)
	{
	case EGameDS_EnemyAIMode::Chase:
		{
			EGameDS_EnemyBattleModeType ChangeableMode;
			if (CheckBattleModeChangeable(ChangeableMode))
			{
				// 추격중에는 스킬을 가지고 있으므로 마지막 스킬 저장
				if (FGameDS_EnemyBattleModeOrderData* CombatOrder = CombatOrderMap.Find(BattleMode))
					CombatOrder->SavedSkillID = NextSkillID;
				
				BattleMode = ChangeableMode;
				UpdateState(EGameDS_EnemyAIMode::ModeTransition);
			}
		}
		break;
	case EGameDS_EnemyAIMode::Attack: break;
	case EGameDS_EnemyAIMode::AttackRest:
		{
			if (RestElapsedTime < RestDuration)
			{
				RestElapsedTime += DeltaSeconds;
			}
			else
			{
				UpdateState(EGameDS_EnemyAIMode::AttackSelection);
			}
		}
		break;
	case EGameDS_EnemyAIMode::FaceOff:
		{
			bool bCancelFaceOff = false;
			bool bPassRestTime = false;

			// 다음 모드로 변경되는 조건
			// 1. 최대 시간
			// 2. 현재 거리 비교 
			if (RestElapsedTime < RestDuration)
			{
				RestElapsedTime += DeltaSeconds;

				// 처음 FaceOff를 시작한 거리 / 2 안쪽으로 접근했을 때 강제로 모드 변경
				const float HalfDist = BeginFaceOffDistance * 0.5f;
				if (GetDistanceToTarget() < HalfDist)
				{
					bCancelFaceOff = true;
					bPassRestTime = true;
				}
			}
			else
			{
				bCancelFaceOff = true;
			}

			if (bCancelFaceOff)
			{
				SetFaceOffMode(false);

				RestElapsedTime = 0.0f;

				if (bPassRestTime)
				{
					UpdateState(EGameDS_EnemyAIMode::AttackSelection);
				}
				else
				{
					const FGameDS_EnemySkillSettingDataTable* SkillSetting = SkillSettingMap.Find(NextSkillID);
					if (SkillSetting == nullptr)
						return;
			
					GetBlackboardComponent()->SetValueAsEnum(TEXT("SkillType"), StaticCast<uint8>(EGameDS_EnemySkillType::Normal));
					RestDuration = SkillSetting->MoveBlockDuration;
					
					UpdateState(EGameDS_EnemyAIMode::AttackRest);
				}
			}
		}
		break;
	case EGameDS_EnemyAIMode::Hit:
		{
			if (bHit)
			{
				if (RestElapsedTime < RestDuration)
				{
					RestElapsedTime += DeltaSeconds;
				}
				else
				{
					RestElapsedTime = 0.0f;
					RestDuration = 0.0f;
				
					bHit = false;
					AnimInstance->SetHit(false);
					UpdateState(EGameDS_EnemyAIMode::AttackSelection);

					ResumeMove(GetCurrentMoveRequestID());
				}
			}
		}
		break;
	default: ;
	}
}

void AGameDS_EnemyAIController::UpdateRotateValue(float DeltaSeconds)
{
	if (AnimInstance == nullptr)
		return;

	if (bHit == true)
		return;

	AnimInstance->SetTurnState(bRotate);
	if (bRotate == false)
		return;

	const FGameDS_EnemySkillSettingDataTable* FindValue = SkillSettingMap.Find(NextSkillID);
	if (FindValue == nullptr)
		return;

	FVector OwnerForwardVector;
	const FVector ToTargetVector = (Target->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal2D();
	if (FindValue->SkillType == EGameDS_EnemySkillType::BackwardSpecial)
	{
		OwnerForwardVector = -OwnerEnemy->GetActorForwardVector().GetSafeNormal2D();
	}
	else
	{
		OwnerForwardVector = OwnerEnemy->GetActorForwardVector().GetSafeNormal2D();
	}
	const FVector CrossResult = FVector::CrossProduct(OwnerForwardVector, ToTargetVector);

	const bool bLeftQuick = CrossResult.Z < 0;

	float CompareToTargetAngle = FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(OwnerForwardVector, ToTargetVector))));
	if (RotateToTargetAngle < CompareToTargetAngle)
	{
		RotateToTargetAngle = CompareToTargetAngle;
	}

	float AngleSpeed = (1.f + (9.f * RotateToTargetAngle / 180.f)) * 30.f; 

	float RotateSpeed = AngleSpeed * FindValue->RotateSpeedFactor;

	if (bLeftQuick)
		RotateSpeed *= -1;

	const FRotator& ResultRotator = FGameDS_Util::ComposeRotator(OwnerEnemy->GetActorRotation(), RotateSpeed, DeltaSeconds);
	FRotator ToTargetRotator = ToTargetVector.Rotation();
	FRotator TestRotator = FRotator::ZeroRotator;
	TestRotator.Yaw = FMath::Abs(ToTargetRotator.Yaw - ResultRotator.Yaw);

	if (FindValue->SkillType == EGameDS_EnemySkillType::BackwardSpecial)
	{
		TestRotator.Yaw -= 180.f;
		ToTargetRotator.Yaw += 180.f;
	}
	
	if (TestRotator.Yaw <= (AngleSpeed * DeltaSeconds))
	{
		OwnerEnemy->SetActorRotation(ToTargetRotator);
		if (bActiveRotate != true)
		{
			bRotate = false;
			AnimInstance->SetTurnState(bRotate);
			RotateToTargetAngle = -1.f;
		}
	}
	else
	{
		AnimInstance->SetTurnLeft(bLeftQuick);
		OwnerEnemy->SetActorRotation(ResultRotator);
	}
}

void AGameDS_EnemyAIController::ChangeBattleModeSetting(bool bActive, AGameDS_HeroCharacter* InTarget/* = nullptr*/)
{
	if (OwnerEnemy == nullptr)
		return;
	
	if (StatConfigData == nullptr)
		return;

	if (bActive)
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), InTarget);
		Target = InTarget;
		
		AnimInstance->ChangeBattleMode(true);

		OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->WalkSpeed;

		// Choose BattleMode
		ChangeBattleModeTypeWithDistance();
		UpdateState(EGameDS_EnemyAIMode::AttackSelection);
	}
	else
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);

		UpdateState(EGameDS_EnemyAIMode::Patrol);

		if (bPatrolUnit)
		{
			OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->PatrolSpeed;
		}
		else
		{
			OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->WalkSpeed;
		}
		
		AnimInstance->ChangeBattleMode(false);
		ChangeBattleModeType(EGameDS_EnemyBattleModeType::NoneBattleMode);
		
		ResetTargetDelegate.ExecuteIfBound();
	}
}

void AGameDS_EnemyAIController::ChangeBattleModeType(EGameDS_EnemyBattleModeType InBattleMode)
{
	BattleMode = InBattleMode;
}

void AGameDS_EnemyAIController::DestroyWithDelay(float InDelay)
{
	DestroyRemainTime = InDelay;
	bActiveDestroy = true;
}

void AGameDS_EnemyAIController::ChangeBattleModeTypeWithDistance()
{

	if (OwnerEnemy == nullptr)
		return;

	if (Target == nullptr)
		return;

	if (StatConfigData == nullptr)
		return;

	if (StatConfigData->BattleModeSettingMap.IsEmpty() || StatConfigData->BattleModeSettingMap.Num() == 1)
	{
		ChangeBattleModeType(StatConfigData->DefaultBattleMode);
		return;
	}

	const float Distance = GetDistanceToTarget();

	for (const auto& BattleModePairData : StatConfigData->BattleModeSettingMap)
	{
		if (BattleMode == BattleModePairData.Key)
			continue;

		const float MinValue = BattleModePairData.Value.BattleModeRange.X;
		const float MaxValue = BattleModePairData.Value.BattleModeRange.Y;

		if (FMath::IsWithin(Distance, MinValue, MaxValue))
		{
			ChangeBattleModeType(BattleModePairData.Key);
			return;
		}
	}
}

bool AGameDS_EnemyAIController::CheckBattleModeChangeable(EGameDS_EnemyBattleModeType& ChangeableMode)
{
	ChangeableMode = EGameDS_EnemyBattleModeType::None;
	
	if (OwnerEnemy == nullptr)
		return false;
	
	if (Target == nullptr)
		return false;

	if (StatConfigData == nullptr)
		return false;

	if (BattleMode == EGameDS_EnemyBattleModeType::BossSecondPhaseMode)
		return false;

	for (const auto& PairData : StatConfigData->BattleModeSettingMap)
	{
		if (PairData.Key == BattleMode)
			continue;

		if (ModeChangeDelayElapsedTime < PairData.Value.ChangeModeDelay)
			continue;

		const float MinValue = PairData.Value.BattleModeRange.X;
		const float MaxValue = PairData.Value.BattleModeRange.Y;
		if (FMath::IsWithin(GetDistanceToTarget(), MinValue, MaxValue) == false)
			continue;

		ChangeableMode = PairData.Key;
		return true;
	}

	return false;
}

void AGameDS_EnemyAIController::ChangeRotateSetting(bool bInSetting)
{
	bActiveRotate = bInSetting;
	bRotate = bInSetting;
}

bool AGameDS_EnemyAIController::CheckAttackRange()
{
	if (AIMode == EGameDS_EnemyAIMode::Attack)
		return false;

	if (OwnerEnemy == nullptr)
		return false;

	if (Target == nullptr)
	{
		UObject* TargetObject = GetBlackboardComponent()->GetValueAsObject(TEXT("Target"));
		if (TargetObject == nullptr)
			return false;

		Target = Cast<AGameDS_HeroCharacter>(TargetObject);
	}

	if (Target == nullptr)
		return false;

	const float TotalDistance = GetDistanceToTarget();
	
	const FGameDS_EnemySkillSettingDataTable* FindValue = SkillSettingMap.Find(NextSkillID);
	if (FindValue == nullptr)
		return false;

	switch (FindValue->SkillType)
	{
	case EGameDS_EnemySkillType::Normal:;
	case EGameDS_EnemySkillType::ForwardSpecial:
	case EGameDS_EnemySkillType::BackwardSpecial:
		{
			return TotalDistance < FindValue->MaxDistance;
		}
	case EGameDS_EnemySkillType::FaceOffSpecial:
		{
			return FindValue->MinDistance < TotalDistance;
		}
	case EGameDS_EnemySkillType::LongDistanceSpecial:
		{
			if (FindValue->MinDistance < TotalDistance)
			{
				return true;
			}
			return false;
		}
	case EGameDS_EnemySkillType::Evade:
	{
		return true;
	}
	default: ;
	}
		
	return false;
}

bool AGameDS_EnemyAIController::CheckAttackAngle()
{
	const FGameDS_EnemySkillSettingDataTable* FindValue = SkillSettingMap.Find(NextSkillID);
	if (FindValue == nullptr)
		return false;

	const FVector Result = (Target->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal2D();
	const FVector Forward2D = OwnerEnemy->GetActorForwardVector().GetSafeNormal2D();
	const FVector Backward2D = -Forward2D;
	
	float ToTargetAngle = FGameDS_Util::GetDifferenceDegree2D(Forward2D, Result);

	if (FindValue->SkillType == EGameDS_EnemySkillType::BackwardSpecial)
	{
		ToTargetAngle = FGameDS_Util::GetDifferenceDegree2D(Backward2D, Result);
		if (ToTargetAngle <= 20.0f)
		{
			bRotate = false;
			return true;
		}
	}
	else
	{
		if (ToTargetAngle <= FindValue->MinAngleDifference)
		{
			bRotate = false;
			return true;
		}
	}
	
	bRotate = true;
	return false;
}

bool AGameDS_EnemyAIController::CheckFaceOffAngle()
{
	if (Target == nullptr)
		return false;

	const FVector Result = (Target->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal2D();
	const FVector Forward2D = OwnerEnemy->GetActorForwardVector().GetSafeNormal2D();
	const float ToTargetAngle = FGameDS_Util::GetDifferenceDegree2D(Forward2D, Result);

	if (ToTargetAngle <= 5.0f)
	{
		bRotate = false;
		return true;
	}

	bRotate = true;
	return false;
}

void AGameDS_EnemyAIController::UpdateState(EGameDS_EnemyAIMode InAIMode)
{
	if (AIMode == InAIMode)
		return;

	if (OwnerEnemy == nullptr)
		return;

	if (StatConfigData == nullptr)
		return;

	if (AIMode == EGameDS_EnemyAIMode::Die)
		return;

	EGameDS_EnemyAIMode BeforeMode = AIMode;
	AIMode = InAIMode;
	GetBlackboardComponent()->SetValueAsEnum(TEXT("AIState"), StaticCast<uint8>(AIMode));

	switch (InAIMode)
	{
	case EGameDS_EnemyAIMode::Attack:
		{
			bRotate = false;
			AnimInstance->PlayMontage(NextSkillID);
		}
		break;
	case EGameDS_EnemyAIMode::AttackRest:
		{
			RestElapsedTime = 0.0f;

			const FGameDS_EnemySkillSettingDataTable* FindValue = SkillSettingMap.Find(NextSkillID);
			if (FindValue == nullptr)
				return;
			
			RestDuration = FindValue->MoveBlockDuration;
		}
		break;
	case EGameDS_EnemyAIMode::FaceOff:
		break;
	case EGameDS_EnemyAIMode::AttackSelection:
		{
			const bool bBoss = GetBlackboardComponent()->GetValueAsBool(TEXT("bBoss"));

			EGameDS_EnemyBattleModeType ChangeableMode;
			if (CheckBattleModeChangeable(ChangeableMode))
			{
				BattleMode = ChangeableMode;
				UpdateState(EGameDS_EnemyAIMode::ModeTransition);

				if (bPlayModeTransitionAnim)
					return;
			}

			if (BeforeHitAIMode == EGameDS_EnemyAIMode::Chase)
			{
				BeforeHitAIMode = EGameDS_EnemyAIMode::None;
				UpdateState(EGameDS_EnemyAIMode::Chase);
				if (bBoss)
				{
					CheckSprintCondition();
				}
				return;
			}
			
			NextSkillType = GetCulNextSkillType();
			if (NextSkillType == EGameDS_EnemySkillType::FaceOffSpecial)
			{
				SetFaceOffMode(true);
				SaveFaceOffDistance();
				UpdateState(EGameDS_EnemyAIMode::FaceOff);
			}
			else if (NextSkillType == EGameDS_EnemySkillType::Normal ||
					NextSkillType == EGameDS_EnemySkillType::ForwardSpecial)
			{
				UpdateState(EGameDS_EnemyAIMode::Chase);

				if (bBoss)
				{
					CheckSprintCondition();
				}
			}
			else if (NextSkillType == EGameDS_EnemySkillType::LongDistanceSpecial)
			{

			}
			GetBlackboardComponent()->SetValueAsEnum(TEXT("SkillType"), StaticCast<uint8>(NextSkillType));
		}
		break;
	case EGameDS_EnemyAIMode::Hit:
		{
			if (bHit)
				return;
			
			const bool bBoss = GetBlackboardComponent()->GetValueAsBool(TEXT("bBoss"));

			if (bBoss == true)
			{
				if(AGameDS_Boss* BossCharacter = Cast<AGameDS_Boss>(OwnerEnemy))
				{
					if(BossCharacter->IsPhaseEnd() == true)
						return;
				}

				if (BeforeMode == EGameDS_EnemyAIMode::AttackRest)
				{
					FGameDS_EnemyDetailSkillData* EvadeData = GetFrontSkillData(EGameDS_EnemySkillType::Evade);

					if (EvadeData->WaitCount-- == 0)
					{
						EvadeData->WaitCount = EvadeData->MaxWaitCount;
						NextSkillID = EvadeData->SkillID;
						UpdateState(EGameDS_EnemyAIMode::Evade);
					}
					else
					{
						bHit = true;
						RestElapsedTime = 0.0f;
						RestDuration = 1.0f;
					}
					return;
				}
			}

			if (bBoss == true || BeforeMode == EGameDS_EnemyAIMode::Attack)
			{
				AIMode = BeforeMode;
				GetBlackboardComponent()->SetValueAsEnum(TEXT("AIState"), StaticCast<uint8>(AIMode));
				return;
			}

			BeforeHitAIMode = BeforeMode;
			
			bHit = true;
			RestElapsedTime = 0.0f;
			RestDuration = 1.0f;
			AnimInstance->SetHit(true);
		}
		break;
	case EGameDS_EnemyAIMode::ModeTransition:
		{
			ModeChangeDelayElapsedTime = 0.0f;
			FGameDS_EnemyDetailSkillData* TransitionSkillData = GetFrontSkillData(EGameDS_EnemySkillType::ModeTransition);
			
			if (BattleMode == EGameDS_EnemyBattleModeType::NoneBattleMode)
			{
				ChangeBattleModeSetting(false);
			}
			else if (TransitionSkillData != nullptr)
			{
				bPlayModeTransitionAnim = true;
				
				PauseMove(GetCurrentMoveRequestID());
				AnimInstance->PlayMontage(TransitionSkillData->SkillID);
			}
		}
		break;
	case EGameDS_EnemyAIMode::PhaseEnd:
		{

		}
		break;
	case EGameDS_EnemyAIMode::Chase:
		{
			if (GetMoveStatus() == EPathFollowingStatus::Paused)
			{
				ResumeMove(GetCurrentMoveRequestID());
			}
		}
	case EGameDS_EnemyAIMode::Evade:
		{
			
		}
		break;
	default: ;
	}
}

FVector AGameDS_EnemyAIController::NextPatrolPoint()
{
	if (OwnerEnemy == nullptr)
		return FVector::ZeroVector;
	
	if (PatrolPoint.IsEmpty())
		return OwnerEnemy->GetActorLocation();
		
	NextPatrolIndex++;

	if (NextPatrolIndex == PatrolPoint.Num())
		NextPatrolIndex = 0;
	
	return PatrolPoint[NextPatrolIndex];
}

FVector AGameDS_EnemyAIController::GetPatrolPoint()
{
	return PatrolPoint[NextPatrolIndex];
}

bool AGameDS_EnemyAIController::RandomEvade()
{
	if (GetDistanceToTarget() < 100.f)
		return false;

	float RandomValue = FMath::FRand();
	bool bIsEvade = RandomValue > 0.5f;
	if (bIsEvade)
	{
		AnimInstance->StopAllMontages(0.5f);
		AnimInstance->SetEvade(bIsEvade);
	}
	return bIsEvade;
}

void AGameDS_EnemyAIController::CalculrateTargetLocation(bool ToCharacter /*= true */, float InDistance/* = 100.f */,float InGap /* = 100.f*/)
{
	if (Target == nullptr)
		return;

	FVector TargetLocation;
	FVector OwnerEnemyLocation = OwnerEnemy->GetActorLocation();
	FVector OwenrEnemyForwardVector = OwnerEnemy->GetActorForwardVector().GetSafeNormal2D(); //(Target->GetActorLocation() - OwnerEnemyLocation).GetSafeNormal2D(); 

	if (ToCharacter == true)
	{
		TargetLocation = Target->GetActorLocation();
	}
	else
	{
		FHitResult Hit;
		TargetLocation = OwnerEnemyLocation + OwenrEnemyForwardVector * InDistance;
		FCollisionQueryParams TraceParams;
		TraceParams.bReturnPhysicalMaterial = true;
		TraceParams.bTraceComplex = false;
		TraceParams.AddIgnoredActor(Target);
		bool bHitComplete = OwnerEnemy->GetWorld()->LineTraceSingleByChannel(
			Hit,
			OwnerEnemyLocation,
			TargetLocation,
			ECC_Visibility,
			TraceParams);
		
		if (bHitComplete == true)
		{
			TargetLocation = Hit.Location;
		}
	}

	FVector ToTargetVector = (TargetLocation - OwnerEnemyLocation).GetSafeNormal2D();

	SavedTargetLocation = TargetLocation - InGap * ToTargetVector;

	FVector ToSavedTargetLocationVector = (SavedTargetLocation - OwnerEnemyLocation).GetSafeNormal2D();

	if (ToCharacter == true)
	{
		float DotResult = FVector::DotProduct(ToSavedTargetLocationVector, OwenrEnemyForwardVector);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(DotResult));

		if (FMath::Abs(Angle) >= 90.f)
		{
			SavedTargetLocation = OwnerEnemyLocation;
		}
	}
}

void AGameDS_EnemyAIController::SetTargetLocation(bool InSetted, float InFollowSpeed, bool InLastMove /*= true */)
{
	bSetTargetLocation = InSetted;
	bLastMoving = InLastMove;

	if (bSetTargetLocation == false)
	{
		FollowSpeed = 0.f;
		SavedTargetLocation = FVector::ZeroVector;
	}
	else
	{
		FollowSpeed = InFollowSpeed;
	}
	return;
}

void AGameDS_EnemyAIController::CachingSkillSetting()
{
	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DataManager == nullptr)
		return;

	if (SkillSettingMap.IsEmpty() == false)
	{
		SkillSettingMap.Reset();
	}

	if (CombatOrderMap.IsEmpty() == false)
	{
		CombatOrderMap.Reset();
	}

	if (AnimInstance != nullptr)
	{
		TArray<EGameDS_UnitAnimType> AllAnimTypeList;
		FGameDS_EnumUtil::GetAllUnitAnimType(AllAnimTypeList);

		for (const EGameDS_UnitAnimType AnimType : AllAnimTypeList)
		{
			TArray<int32> InnerSkillIDList;
			AnimInstance->GetSkillID(AnimType, InnerSkillIDList);

			for (int32 SkillID : InnerSkillIDList)
			{
				const FGameDS_EnemySkillSettingDataTable* SettingData = DataManager->GetDataInRow<FGameDS_EnemySkillSettingDataTable>(SkillID);
				if (SettingData == nullptr)
					continue;
				
				const FGameDS_SkillDataTable* SkillData = DataManager->GetDataInRow<FGameDS_SkillDataTable>(SkillID);
				if (SkillData == nullptr)
					continue;
				
				TArray<FGameDS_EnemyDetailSkillData>& DetailSkillList = GetDetailSkillList(SettingData->BattleType, SettingData->SkillType);
				
				FGameDS_EnemyDetailSkillData DetailSkillData;
				DetailSkillData.WaitCount = SettingData->WaitCount;
				DetailSkillData.MaxWaitCount = SettingData->WaitCount;
				DetailSkillData.bMontageSkill = SkillData->SkillMontage != nullptr;
				DetailSkillData.SkillID = SkillID;
				
				DetailSkillList.Add(DetailSkillData);
				SkillSettingMap.Add(SkillID, *SettingData);
			}
		}
	}
}

float AGameDS_EnemyAIController::GetCalculateRotateValue()
{
	const FVector ToTargetVector = (Target->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal2D();

	FVector CrossResult = FVector::CrossProduct(OwnerEnemy->GetActorForwardVector(), ToTargetVector);
	bool bLeftQuick = CrossResult.Z < 0;
	
	const FGameDS_EnemySkillSettingDataTable* FindValue = SkillSettingMap.Find(NextSkillID);
	if (FindValue == nullptr)
		return 0.0f;

	float AdditionalRotateValue = 100 * FindValue->RotateSpeedFactor;
	if (bLeftQuick)
		AdditionalRotateValue *= -1;

	return AdditionalRotateValue;
}

EGameDS_EnemySkillType AGameDS_EnemyAIController::GetCulNextSkillType()
{
	if (OwnerEnemy == nullptr)
		return EGameDS_EnemySkillType::None;

	if (Target == nullptr)
	{
		UObject* TargetObject = GetBlackboardComponent()->GetValueAsObject(TEXT("Target"));
		if (TargetObject == nullptr)
			return EGameDS_EnemySkillType::None;

		Target = Cast<AGameDS_HeroCharacter>(TargetObject);
		
		if (Target == nullptr)
			return EGameDS_EnemySkillType::None;
	}

	if (FGameDS_EnemyBattleModeOrderData* CombatOrder = CombatOrderMap.Find(BattleMode))
	{
		if (CombatOrder->SavedSkillID != INDEX_NONE)
		{
			NextSkillID = CombatOrder->SavedSkillID;
			CombatOrder->SavedSkillID = INDEX_NONE;

			if (const FGameDS_EnemySkillSettingDataTable* NextSkillData = SkillSettingMap.Find(NextSkillID))
				return NextSkillData->SkillType;
		}
	}

	const FVector ToTargetUnitVector = (Target->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal2D();
	const float ToTargetDist = FVector::Dist(OwnerEnemy->GetActorLocation(), Target->GetActorLocation());
	const float ToTargetDegree = FGameDS_Util::GetDifferenceDegree2D(OwnerEnemy->GetActorForwardVector(), ToTargetUnitVector);
	const float ToTargetReverseDegree = FGameDS_Util::GetDifferenceDegree2D(-OwnerEnemy->GetActorForwardVector(), ToTargetUnitVector);

	FGameDS_EnemyDetailSkillData* FaceOffSpecialData = GetFrontSkillData(EGameDS_EnemySkillType::FaceOffSpecial);
	FGameDS_EnemyDetailSkillData* BackwardSpecialData = GetFrontSkillData(EGameDS_EnemySkillType::BackwardSpecial);
	FGameDS_EnemyDetailSkillData* ForwardSpecialData = GetFrontSkillData(EGameDS_EnemySkillType::ForwardSpecial);
	FGameDS_EnemyDetailSkillData* NormalData = GetFrontSkillData(EGameDS_EnemySkillType::Normal);
	FGameDS_EnemyDetailSkillData* LongDistanceSpecialData = GetFrontSkillData(EGameDS_EnemySkillType::LongDistanceSpecial);

	// FaceOff Check
	if (FaceOffSpecialData != nullptr)
	{
		if (const FGameDS_EnemySkillSettingDataTable* SettingData = SkillSettingMap.Find(FaceOffSpecialData->SkillID))
		{
			const bool bDistanceFlag = SettingData->MinDistance <= ToTargetDist;
		
			if (bDistanceFlag)
			{
				if (FaceOffSpecialData->WaitCount--== 0)
				{
					FaceOffSpecialData->WaitCount = FaceOffSpecialData->MaxWaitCount;
					NextSkillID = FaceOffSpecialData->SkillID;
					NextOrder(EGameDS_EnemySkillType::FaceOffSpecial);
				
					return EGameDS_EnemySkillType::FaceOffSpecial;
				}
			}
		}
	}

	// Backward Check
	if (BackwardSpecialData != nullptr)
	{
		if (const FGameDS_EnemySkillSettingDataTable* SettingData = SkillSettingMap.Find(BackwardSpecialData->SkillID))
		{
			const bool bDistanceFlag = ToTargetDist <= SettingData->MaxDistance;
			const bool bDegreeFlag = ToTargetReverseDegree <= SettingData->MinAngleDifference;
			
			if (bDistanceFlag && bDegreeFlag)
			{
				if (BackwardSpecialData->WaitCount-- == 0)
				{
					BackwardSpecialData->WaitCount = BackwardSpecialData->MaxWaitCount;
					NextSkillID = BackwardSpecialData->SkillID;
					NextOrder(EGameDS_EnemySkillType::BackwardSpecial);
				
					return EGameDS_EnemySkillType::BackwardSpecial;
				}
			}
		}
	}

	// Forward Check
	if (ForwardSpecialData != nullptr)
	{
		if (const FGameDS_EnemySkillSettingDataTable* SettingData = SkillSettingMap.Find(ForwardSpecialData->SkillID))
		{
			if (ForwardSpecialData->WaitCount-- <= 0)
			{
				const bool bDistanceFlag = SettingData->MinDistance <= ToTargetDist;
				if (bDistanceFlag)
				{
					ForwardSpecialData->WaitCount = ForwardSpecialData->MaxWaitCount;
					NextSkillID = ForwardSpecialData->SkillID;
					NextOrder(EGameDS_EnemySkillType::ForwardSpecial);

					return EGameDS_EnemySkillType::ForwardSpecial;
				}
			}
		}
	}
	
	// LongDistanceSpeicial
	if (LongDistanceSpecialData != nullptr)
	{
		if (const FGameDS_EnemySkillSettingDataTable* SettingData = SkillSettingMap.Find(LongDistanceSpecialData->SkillID))
		{
			if (LongDistanceSpecialData->WaitCount-- <= 0)
			{
				const bool bDistanceFlag = SettingData->MinDistance <= ToTargetDist;
				LongDistanceSpecialData->WaitCount = LongDistanceSpecialData->MaxWaitCount;

				if (bDistanceFlag)
				{
					NextSkillID = LongDistanceSpecialData->SkillID;
					NextOrder(EGameDS_EnemySkillType::LongDistanceSpecial);
					return EGameDS_EnemySkillType::LongDistanceSpecial;
				}
				else
				{
					NextSkillID = 319;
					NextOrder(EGameDS_EnemySkillType::ForwardSpecial);
					
					if (FGameDS_EnemyBattleModeOrderData* CombatOrder = CombatOrderMap.Find(BattleMode))
						CombatOrder->SavedSkillID = LongDistanceSpecialData->SkillID;

					return EGameDS_EnemySkillType::ForwardSpecial;
				}
			}
		}
	}

	// Normal Check
	if (NormalData != nullptr)
	{
		NextSkillID = NormalData->SkillID;
		NextOrder(EGameDS_EnemySkillType::Normal);

		return EGameDS_EnemySkillType::Normal;
	}

	return EGameDS_EnemySkillType::None;
}

TArray<FGameDS_EnemyDetailSkillData>& AGameDS_EnemyAIController::GetDetailSkillList(EGameDS_EnemyBattleModeType InBattleModeType, EGameDS_EnemySkillType InSkillType)
{
	FGameDS_EnemyBattleModeOrderData* BattleModeOrderData = CombatOrderMap.Find(InBattleModeType);
	if (BattleModeOrderData == nullptr)
		BattleModeOrderData = &CombatOrderMap.Add(InBattleModeType, FGameDS_EnemyBattleModeOrderData());

	FGameDS_EnemySkillOrderData* CombatOrderData = BattleModeOrderData->CombatOrder.Find(InSkillType);
	if (CombatOrderData == nullptr)
		BattleModeOrderData->CombatOrder.Add(InSkillType, FGameDS_EnemySkillOrderData());
	
	return BattleModeOrderData->CombatOrder.Find(InSkillType)->DetailSkillDataList;
}

FGameDS_EnemyDetailSkillData* AGameDS_EnemyAIController::GetFrontSkillData(EGameDS_EnemySkillType InSkillType)
{
	FGameDS_EnemyBattleModeOrderData* BattleModeOrderData = CombatOrderMap.Find(BattleMode);
	if (BattleModeOrderData == nullptr)
		return nullptr;
	
	FGameDS_EnemySkillOrderData* OrderData = BattleModeOrderData->CombatOrder.Find(InSkillType);
	if (OrderData == nullptr)
		return nullptr;

	if (OrderData->DetailSkillDataList.IsValidIndex(OrderData->OrderIndex) == false)
		return nullptr;
	
	return &OrderData->DetailSkillDataList[OrderData->OrderIndex];
}

void AGameDS_EnemyAIController::NextOrder(EGameDS_EnemySkillType InSkillType)
{
	FGameDS_EnemyBattleModeOrderData* BattleModeOrderData = CombatOrderMap.Find(BattleMode);
	if (BattleModeOrderData == nullptr)
		return;
	
	FGameDS_EnemySkillOrderData* OrderData = BattleModeOrderData->CombatOrder.Find(InSkillType);
	if (OrderData == nullptr)
		return;


	int32 NextOrder = OrderData->OrderIndex + 1;
	if (NextOrder == OrderData->DetailSkillDataList.Num())
		NextOrder = 0;
	OrderData->OrderIndex = NextOrder;

	/* Random Order
	 int SkillNumsOfSkillType = OrderData->DetailSkillDataList.Num();

	 if (SkillNumsOfSkillType == 1)
	 	return;

	 while (true)
	 {
	 	int RandIndex = FMath::Rand() % SkillNumsOfSkillType;

	 	if (OrderData->OrderIndex != RandIndex)
	 	{
	 		OrderData->OrderIndex = RandIndex;
	 		break;
	 	}
	}
	*/
}

void AGameDS_EnemyAIController::FollowTarget(float DeltaTime)
{
	FVector NextLocation = FMath::VInterpTo(OwnerEnemy->GetActorLocation(), SavedTargetLocation, DeltaTime, FollowSpeed);
	OwnerEnemy->SetActorLocation(NextLocation);
	float Distance = FVector::Dist2D(NextLocation, SavedTargetLocation);

	if (FMath::Abs(Distance) < FollowSpeed)
	{
		bSetTargetLocation = false;
		if (bLastMoving == true)
		{
			uint8 TempSectionNum = AnimInstance->GetMontageBySkillID(NextSkillID)->GetNumSections();
			if (TempSectionNum > 2)
				AnimInstance->Montage_JumpToSection(FName("1"));
		}
	}
}

void AGameDS_EnemyAIController::SetFaceOffMode(bool bEnable)
{
	const FGameDS_EnemySkillSettingDataTable* SkillSetting = SkillSettingMap.Find(NextSkillID);

	if (SkillSetting == nullptr)
		return;
	
	if (StatConfigData == nullptr)
		return;

	if (bEnable)
	{
		OwnerEnemy->GetCharacterMovement()->bOrientRotationToMovement = false;
		AnimInstance->SetLookAt(true);
		const FVector LookAtVector = (Target->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal2D();
		const FRotator LookAtRotator = LookAtVector.Rotation();
		const float ToTargetAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(OwnerEnemy->GetActorForwardVector().GetSafeNormal2D(), LookAtVector)));
		RestDuration = SkillSetting->MoveBlockDuration;
		OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->FaceOffSpeed;
		if (ToTargetAngle <= 1.f)
		{
			OwnerEnemy->SetActorRotation(LookAtRotator);
		}
	}
	else
	{
		OwnerEnemy->GetCharacterMovement()->bOrientRotationToMovement = true;
		AnimInstance->SetLookAt(false);
		RestDuration = 0.0f;
		OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->WalkSpeed;
	}
	RestElapsedTime = 0.0f;
}

void AGameDS_EnemyAIController::SaveFaceOffDistance()
{
	BeginFaceOffDistance = GetDistanceToTarget();
}

float AGameDS_EnemyAIController::GetDistanceToTarget() const
{
	if (OwnerEnemy == nullptr)
		return 0.0f;

	if (Target == nullptr)
		return 0.0f;
	
	return FVector::Dist2D(OwnerEnemy->GetActorLocation(), Target->GetActorLocation());
}

bool AGameDS_EnemyAIController::IsInRangeChasableDistance() const
{
	if (Target == nullptr)
		return false;

	if (StatConfigData == nullptr)
		return false;

	return GetDistanceToTarget() < StatConfigData->MaximumChasableDistance;
}

void AGameDS_EnemyAIController::CheckSprintCondition()
{
	UGameDS_BossAnimInstance* BossAnimInstance = Cast<UGameDS_BossAnimInstance>(AnimInstance);
	if (BossAnimInstance == nullptr)
		return;
	
	if (StatConfigData == nullptr)
		return;

	if (GetDistanceToTarget() >= 1000.f)
	{
		BossAnimInstance->SetRun(true);
		OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->SprintSpeed;
	}
	else
	{
		BossAnimInstance->SetRun(false);
		OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = StatConfigData->WalkSpeed;
	}
}
