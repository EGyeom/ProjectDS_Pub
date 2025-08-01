#include "GameDS_UnitSpawner.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Components/SplineComponent.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "Engine/GameInstance.h"

AGameDS_UnitSpawner::AGameDS_UnitSpawner()
{
	PatrolSplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("SplinePath"));
	PatrolSplinePath->SetupAttachment(GetRootComponent());
}

void AGameDS_UnitSpawner::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGameDS_UnitSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance()))
	{
		FGameDS_UnitSpawnOption SpawnOption;
		SpawnOption.UnitType = UnitType;
		SpawnOption.UnitID = UnitManager->GetNextUnitID();
		SpawnOption.DataID = DataID;
		SpawnOption.Location = GetActorLocation();
		SpawnOption.Rotator = GetActorRotation();
		SpawnOption.bBattleMode = bBattleMode;

		if (bUsePatrol == true)
		{
			SpawnOption.bUsePatrol = bUsePatrol;

			TArray<FVector>& SplinePathRef = SpawnOption.SplinePathPoint;

			const int32 SplinePointNum = PatrolSplinePath->GetNumberOfSplinePoints();
			SplinePathRef.SetNum(SplinePointNum);
			
			for (int32 PointIndex = 0; PointIndex < SplinePointNum; PointIndex++)
			{
				SplinePathRef[PointIndex] = PatrolSplinePath->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
			}
		}

		UnitManager->SpawnCharacter(SpawnOption);
	}
}

void AGameDS_UnitSpawner::CallRetargetPathAndDraw()
{
	TArray<FVector> SplinePath;

	const int32 SplinePointNum = PatrolSplinePath->GetNumberOfSplinePoints();
	SplinePath.SetNum(SplinePointNum);
			
	for (int32 PointIndex = 0; PointIndex < SplinePointNum; PointIndex++)
	{
		SplinePath[PointIndex] = PatrolSplinePath->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
	}

	TArray<TArray<FVector>> ResultPath;
	ResultPath.SetNum(SplinePath.Num());

	for (int32 PathIndex = 1; PathIndex < SplinePath.Num(); PathIndex++)
	{
		auto StartLoc = SplinePath[PathIndex-1];
		auto EndLoc = SplinePath[PathIndex];
		auto CurIndexPath = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), StartLoc, EndLoc);

		if (CurIndexPath == nullptr)
			return;
		
		ResultPath[PathIndex] = CurIndexPath->PathPoints;
	}

	for (int32 PathIndex = 0; PathIndex < ResultPath.Num(); PathIndex++)
	{
		for (int32 PointIndex = 1; PointIndex < ResultPath[PathIndex].Num(); PointIndex++)
		{
			FVector From = ResultPath[PathIndex][PointIndex - 1];
			FVector To = ResultPath[PathIndex][PointIndex];
			DrawDebugLine(GetWorld(), From, To, FColor::Blue, false, 5.0f, 0, 1.5f);
		}
	}
}
