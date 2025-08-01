#include "GameDS_CrowdControlController.h"

#include "CollisionDebugDrawingPublic.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Unit/Enemy/GameDS_EnemyAIController.h"
#include "ProjectDS/Util/GameDS_Util.h"

void UGameDS_CrowdControlController::Init(AGameDS_Character* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
}

void UGameDS_CrowdControlController::Tick(float DeltaTime)
{
	if (CrowdControlList.IsEmpty())
		return;

	UpdateCrowdControl(DeltaTime);
}

void UGameDS_CrowdControlController::ApplyCrowdControl(const TArray<int32>& InCrowdControlIDList, AActor* Caster, FVector CollisionSpawnPoint)
{
	if (OwnerCharacter == nullptr)
		return;

	if (Caster == nullptr)
		return;

	if (InCrowdControlIDList.IsEmpty())
		return;

	if (OwnerCharacter->GetUnitType() == EGameDS_UnitType::Boss)
		return;

	if (OwnerCharacter->IsDied() == true)
		return;

	AGameDS_HeroCharacter* HeroCharacter = Cast<AGameDS_HeroCharacter>(OwnerCharacter);
	if (HeroCharacter != nullptr && HeroCharacter->GetEvading() == true)
	{
		return;
	}

	UGameDS_DataManager* DataManager = OwnerCharacter->GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	for (const int32 CrowdControlID : InCrowdControlIDList)
	{
		auto CrowdControlRow = DataManager->GetDataInRow<FGameDS_CrowdControlInfoDataTable>(CrowdControlID);
		if (CrowdControlRow == nullptr)
			continue;

		FGameDS_CrowdControlDetailData DetailData;
		DetailData.CrowdControlData = CrowdControlRow->CrowdControlData;

		const FGameDS_CrowdControlData& CCData = DetailData.CrowdControlData;

		if (CCData.MoveBlockDuration > 0)
		{
			if (HeroCharacter != nullptr)
			{
				if (HeroCharacter->GetDefesning())
					continue;
			}
			ChangeBlockState(EGameDS_BlockType::MoveBlock, true);
			DetailData.DestLocation = OwnerCharacter->GetActorLocation();

			const FVector CasterToTarget = (OwnerCharacter->GetActorLocation() - Caster->GetActorLocation()).GetSafeNormal();
			const FVector CasterToTarget2D = CasterToTarget.GetSafeNormal2D();

			// Summon의 경우 특이 케이스, 바닥이 띄워져있는 방식이라 HalfHeight만큼 Z에 더해줘야 올바른 방향이 된다.
			FVector SummonToTarget = (OwnerCharacter->GetActorLocation() - CollisionSpawnPoint).GetSafeNormal2D();
			
			OwnerCharacter->PlayCCMotion(CCData.CrowdControlType);

			bool bReqRotate = CCData.CrowdControlType == EGameDS_CrowdControlType::NormalKnockBack ||
				CCData.CrowdControlType == EGameDS_CrowdControlType::HeavyKnockBack;

			bReqRotate = bReqRotate && HeroCharacter != nullptr;
			
			switch (CCData.CrowdControlMoveType)
			{
				case EGameDS_CrowdControlMoveType::CasterToTarget:
					{
						DetailData.DestLocation += CasterToTarget * CCData.Distance;

						if (bReqRotate)
						{
							HeroCharacter->SetActorRotationWithForce((-CasterToTarget2D).Rotation());
						}
					}
					break;
				case EGameDS_CrowdControlMoveType::TargetToCaster:
					{
						DetailData.DestLocation += -CasterToTarget * CCData.Distance;

						if (bReqRotate)
							HeroCharacter->SetActorRotationWithForce(CasterToTarget2D.Rotation());
					}
					break;
				case EGameDS_CrowdControlMoveType::SummonToTarget:
					{
						DetailData.DestLocation.Z += OwnerCharacter->GetDefaultHalfHeight();
						DetailData.DestLocation += SummonToTarget * CCData.Distance;
						
						if (bReqRotate)
							HeroCharacter->SetActorRotationWithForce((-SummonToTarget).Rotation());
					}
					break;
				case EGameDS_CrowdControlMoveType::SummonToCaster:
					{
						DetailData.DestLocation.Z += OwnerCharacter->GetDefaultHalfHeight();
						DetailData.DestLocation += -SummonToTarget * CCData.Distance;
						
						if (bReqRotate)
							HeroCharacter->SetActorRotationWithForce(SummonToTarget.Rotation());
					}
					break;
				default: ;
			}
			
			const float DistanceValue = CCData.Distance * METER_TO_CENTIMETER;
			DetailData.InterpSpeed = DistanceValue / CCData.MoveBlockDuration;
		}
		
		CrowdControlList.Add(DetailData);
	}
}

void UGameDS_CrowdControlController::UpdateCrowdControl(float DeltaTime)
{
	if (OwnerCharacter == nullptr)
		return;
	
	if (OwnerCharacter->IsDied() == true)
		return;

	int32 ActionBlockCount = 0;
	int32 MoveBlockCount = 0;
	int32 RotateBlockCount = 0;
	
	for (int32 Index = CrowdControlList.Num() - 1; Index >= 0; Index--)
	{
		FGameDS_CrowdControlDetailData& CrowdControlDetailInfo = CrowdControlList[Index];
		FGameDS_CrowdControlData& CrowdControlInfo = CrowdControlDetailInfo.CrowdControlData;
		
		if (CrowdControlInfo.MoveBlockDuration > 0.0f)
		{
			CrowdControlInfo.MoveBlockDuration -= DeltaTime;
			MoveBlockCount++;

			FVector Dest = FMath::InterpEaseOut(
				OwnerCharacter->GetActorLocation(),
				CrowdControlDetailInfo.DestLocation,
				DeltaTime,
				CrowdControlDetailInfo.InterpSpeed);

			// nav를 검사하여 정상적인 위치로 fix
			auto NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(OwnerCharacter->GetWorld(), OwnerCharacter->GetActorLocation(), Dest);
			if (NavPath && NavPath->PathPoints.IsEmpty() == false)
			{
				const FVector GoalLoc = NavPath->GetPath()->GetGoalLocation();

				auto A = (Dest - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				auto B = (GoalLoc - OwnerCharacter->GetActorLocation()).GetSafeNormal();

				// Fix가 너무 많이 됬을 때는 넘어간다 (낭떨어지 같은 부분)
				if (FGameDS_Util::GetDifferenceDegree(A, B) <= 15.0f &&
					FMath::Abs(A.Size() - B.Size()) < 20.0f)
				{
					Dest = GoalLoc + FVector::UpVector * OwnerCharacter->GetDefaultHalfHeight();
				}
			}
			
			OwnerCharacter->SetActorLocation(Dest);
		}
		else
		{
			CrowdControlList.RemoveAt(Index);
		}
	}

	if (ActionBlockCount == 0)
		ChangeBlockState(EGameDS_BlockType::ActionBlock, false);
	if (MoveBlockCount == 0)
		ChangeBlockState(EGameDS_BlockType::MoveBlock, false);
	if (RotateBlockCount == 0)
		ChangeBlockState(EGameDS_BlockType::RotateBlock, false);
}

bool UGameDS_CrowdControlController::CheckBlockType(EGameDS_BlockType InBlockType) const
{
	switch (InBlockType)
	{
		case EGameDS_BlockType::MoveBlock:
			return bMoveBlock;
		case EGameDS_BlockType::ActionBlock:
			return bActionBlock;
		case EGameDS_BlockType::RotateBlock:
			return bRotateBlock;
		default: ;
	}
	
	return false;
}

void UGameDS_CrowdControlController::ChangeBlockState(EGameDS_BlockType InBlockType, bool bFlag)
{
	switch (InBlockType)
	{
	case EGameDS_BlockType::MoveBlock:
		bMoveBlock = bFlag;
		break;
	case EGameDS_BlockType::ActionBlock:
		bActionBlock = bFlag;
		break;
	case EGameDS_BlockType::RotateBlock:
		bRotateBlock = bFlag;
		break;
	default: ;
	}
}

void UGameDS_CrowdControlController::RemoveAllCrowdControl()
{
	CrowdControlList.Empty();
}

bool UGameDS_CrowdControlController::IsPlayingCCMotion()
{
	if (CrowdControlList.IsEmpty())
		return false;
	
	for (const FGameDS_CrowdControlDetailData& Data : CrowdControlList)
	{
		if (Data.CrowdControlData.CrowdControlType == EGameDS_CrowdControlType::NormalKnockBack ||
			Data.CrowdControlData.CrowdControlType == EGameDS_CrowdControlType::HeavyKnockBack)
		{
			return true;
		}
	}

	return false;
}
