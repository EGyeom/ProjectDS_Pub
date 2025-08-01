#include "GameDS_DebugManager.h"

#include "Components/LineBatchComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/UI/Debug/GameDS_UnitInfoWidget.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"

void UGameDS_DebugManager::Tick(float DeltaTime, UGameInstance* InGameInstance)
{
	if (bActiveDebug == false)
		return;
	
	if (TickDelay < MaximumTickDelay)
	{
		TickDelay += DeltaTime;
		return;
	}
	TickDelay = 0.0f;
	
	UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(InGameInstance);
	if (UnitManager == nullptr)
		return;

	TArray<int32> RemoveVisibleIDList;
	TArray<int32> RemoveUnVisibleIDList;
	
	for (auto PairData : VisibleUnitInfoMap)
	{
		int32 UnitID = PairData.Key;
		UGameDS_UnitInfoWidget* UnitInfoWidget = PairData.Value;

		if (UnitManager->GetCharacter(UnitID) == nullptr)
		{
			RemoveVisibleIDList.Add(UnitID);
			continue;
		}
			
		if (UnitInfoWidget == nullptr)
			continue;
		
		float Dist = UnitManager->GetDistFromUser(UnitID);
		if (Dist > DebugDistance)
		{
			UnitInfoWidget->SetVisibility(ESlateVisibility::Hidden);
			
			UnVisibleUnitInfoMap.Add(PairData);
			RemoveVisibleIDList.Add(UnitID);
		}
	}

	for (auto PairData : UnVisibleUnitInfoMap)
	{
		int32 UnitID = PairData.Key;
		UGameDS_UnitInfoWidget* UnitInfoWidget = PairData.Value;
		
		if (UnitManager->GetCharacter(UnitID) == nullptr)
		{
			RemoveUnVisibleIDList.Add(UnitID);
			continue;
		}
		
		if (UnitInfoWidget == nullptr)
			continue;
		
		float Dist = UnitManager->GetDistFromUser(UnitID);
		if (Dist <= DebugDistance)
		{
			UnitInfoWidget->SetVisibility(ESlateVisibility::Visible);
			
			VisibleUnitInfoMap.Add(PairData);
			RemoveUnVisibleIDList.Add(UnitID);
		}
	}

	for (int32 UnitID : RemoveVisibleIDList)
		VisibleUnitInfoMap.Remove(UnitID);
	
	for (int32 UnitID : RemoveUnVisibleIDList)
		UnVisibleUnitInfoMap.Remove(UnitID);
}

void UGameDS_DebugManager::SetDebugMode(UGameInstance* InGameInstance)
{
	if (InGameInstance == nullptr)
		return;

	bActiveDebug = !bActiveDebug;

	if (bActiveDebug)
	{
		InitDebugSetting(InGameInstance);
	}
	else
	{
		for (auto PairData : VisibleUnitInfoMap)
		{
			if (auto UnitInfoWidget = PairData.Value)
				UnitInfoWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		
		VisibleUnitInfoMap.Empty();
		UnVisibleUnitInfoMap.Empty();
	}
}

void UGameDS_DebugManager::InitDebugSetting(UGameInstance* InGameInstance)
{
	if (InGameInstance == nullptr)
		return;

	UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(InGameInstance);
	if (UnitManager == nullptr)
		return;

	TArray<AGameDS_Enemy*> AllEnemyCharacter;
	UnitManager->GetAllCharacter<AGameDS_Enemy>(AllEnemyCharacter);

	for (auto UnitData : AllEnemyCharacter)
	{
		if (UnitData == nullptr)
			continue;

		UGameDS_UnitInfoWidget* InfoWidget = UnitData->GetUnitInfoWidget();
		if (InfoWidget == nullptr)
			continue;

		float Dist = UnitManager->GetDistFromUser(UnitData->GetUnitID());

		if (Dist <= DebugDistance)
		{
			InfoWidget->SetVisibility(ESlateVisibility::Visible);
			InfoWidget->InitInfoSetting(UnitData);
			
			VisibleUnitInfoMap.Add(UnitData->GetUnitID(), InfoWidget);
		}
		else
		{
			InfoWidget->SetVisibility(ESlateVisibility::Hidden);
			UnVisibleUnitInfoMap.Add(UnitData->GetUnitID(), InfoWidget);
		}
	}

	TickDelay = MaximumTickDelay + 0.01f; 
}

void UGameDS_DebugManager::SetDebugDistance(float InDistanceValue)
{
	DebugDistance = FMath::Max(0.0f, InDistanceValue);
}

void UGameDS_DebugManager::SetCollisionDuration(float InCollisionDuration)
{
	CollisionDuration = FMath::Max(0.0f, InCollisionDuration);
}

void UGameDS_DebugManager::MakeDebugCollision(AGameDS_Character* Owner, const FGameDS_CollisionInfoDataTable* CollisionInfo, bool bSummon, FVector SummonDirection, FVector SummonLocation, bool Hit)
{
	MakeDebugCollision(Owner, CollisionInfo, Hit, bSummon, SummonDirection, SummonLocation);
}

void UGameDS_DebugManager::MakeDebugCollision(AGameDS_Character* Owner, const FGameDS_CollisionInfoDataTable* CollisionInfo, bool Hit, bool bSummon, FVector SummonDirection, FVector SummonLocation)
{
	if (bActiveDebug == false)
		return;
	
	if (Owner == nullptr)
		return;
	
	if (CollisionInfo == nullptr)
		return;
	/*
	** FGameDS_CollisionInfoDataTable 도형별 파라미터 **
	@ Box
		Param0 = Forward Distance
		Param1 = ForwardRange
		Param2 = Width
		Param3 = Height
	
	@ Sphere
		Param0 = Forward Distance
		Param1 = ForwardRange
		Param2 = Radius
	
	@ Capsule
		Param0 = Forward Distance
		Param1 = ForwardRange
		Param2 = Radius
	*/
	
	FVector SpawnPoint = FVector::ZeroVector;
	if (bSummon == false)
	{
		if (CollisionInfo->CollisionSpawnType == EGameDS_SkillSpawnType::CasterForward)
		{
			SpawnPoint = Owner->GetActorLocation();
			SpawnPoint += Owner->GetActorForwardVector() * CollisionInfo->Param0;
		}
		else if (CollisionInfo->CollisionSpawnType == EGameDS_SkillSpawnType::WeaponSocket)
		{
			const AGameDS_HeroCharacter* HeroCharacter = Cast<AGameDS_HeroCharacter>(Owner);
			SpawnPoint = HeroCharacter->GetMesh()->GetSocketLocation(HeroCharacter->GetWeaponSocketName());
		}
		else if (CollisionInfo->CollisionSpawnType == EGameDS_SkillSpawnType::Self)
		{
			SpawnPoint = Owner->GetActorLocation();
		}
	}
	else
	{
		SpawnPoint = SummonLocation;
	}

	if (SpawnPoint == FVector::ZeroVector)
		return;
		
	FRotator Direction = Owner->GetActorForwardVector().Rotation();
	if (bSummon)
		Direction = SummonDirection.Rotation();
	
	UWorld* World = Owner->GetWorld();
	
	FColor HitColor = Hit ? FColor::Green : FColor::Red;
	switch (CollisionInfo->CollisionShape)
	{
	case EGameDS_CollisionShape::Box:
    {
		const FVector BoxSize = FVector(CollisionInfo->Param1, CollisionInfo->Param2, CollisionInfo->Param3);
		DrawDebugBox(World, SpawnPoint, BoxSize, FQuat(Direction), HitColor, false, CollisionDuration);
	}
	break;
	case EGameDS_CollisionShape::Sphere:
	{
		DrawDebugSphere(World, SpawnPoint, CollisionInfo->Param2, 10, HitColor, false, CollisionDuration);
	}
	break;
	case EGameDS_CollisionShape::Capsule:
	{
		DrawDebugCapsule(World, SpawnPoint, CollisionInfo->Param2, CollisionInfo->Param1, FQuat(Direction), HitColor, false, CollisionDuration);
	}
	break;
	case EGameDS_CollisionShape::Cylinder:
	{
		/*
		@ Cylinder
		Param0 = Forward Distance
		Param1 = height
		Param2 = radius
		param3 = angle
		param4 = rotate
		 */

		DrawDebugCylinderWithAngle(World, SpawnPoint, CollisionInfo->Param1 * 0.5f, CollisionInfo->Param2, CollisionInfo->Param3, Direction.Vector(), CollisionInfo->Param4, HitColor);
	}
	default: ;
	}
}

void UGameDS_DebugManager::DrawDebugCylinderWithAngle(UWorld* InWorld, FVector CenterPoint, float HalfHeight,
	float Radius, float Angle, FVector Direction, float RotatePitchValue, FColor HitColor)
{
	TArray<float> AngleList{ 0.0f };
	float CurAngle = 0.0f;
	float HalfAngle = Angle * 0.5f;
	while (CurAngle + 9.99f < HalfAngle)
	{
		CurAngle += 10.0f;
		if (CurAngle > HalfAngle)
			CurAngle = FMath::Max(CurAngle, HalfAngle);
		
		AngleList.Add(CurAngle);
	}

	const FVector TopStartPoint = FVector::UpVector * HalfHeight;
	const FVector BottomStartPoint = -FVector::UpVector * HalfHeight;
	
	FVector LastLeftTopPoint;
	FVector LastLeftBottomPoint;
	FVector LastRightTopPoint;
	FVector LastRightBottomPoint;

	TArray<FLineDrawSample> LineData;

	for (int32 Index = 0; Index < AngleList.Num(); Index++)
	{
		if (Index == 0)
		{
			Draw2DBoxSave(LineData,
				BottomStartPoint,
				TopStartPoint,
				TopStartPoint + (Direction * Radius),
				BottomStartPoint + (Direction * Radius),
				false);

			LastLeftTopPoint = TopStartPoint + (Direction * Radius);
			LastLeftBottomPoint = BottomStartPoint + (Direction * Radius);
			LastRightTopPoint = LastLeftTopPoint;
			LastRightBottomPoint = LastLeftBottomPoint;
		}
		else
		{
			// left
			const float InAngle = AngleList[Index];

			float X = Direction.X * UKismetMathLibrary::DegCos(InAngle) - Direction.Y * UKismetMathLibrary::DegSin(InAngle);
			float Y = Direction.X * UKismetMathLibrary::DegSin(InAngle) + Direction.Y * UKismetMathLibrary::DegCos(InAngle);
			FVector LeftTopResultPoint = TopStartPoint + FVector(X,Y,0.0f) * Radius;
			FVector LeftBottomResultPoint = BottomStartPoint + FVector(X,Y,0.0f) * Radius;
			
			Draw2DBoxSave(LineData,
				BottomStartPoint,
				TopStartPoint,
				LeftTopResultPoint,
				LeftBottomResultPoint,
				true);

			// 이전 포인트와 연결 (top)
			DrawLineSave(LineData,
				LeftTopResultPoint,
				LastLeftTopPoint);

			// 이전 포인트와 연결 (bottom)
			DrawLineSave(LineData,
				LeftBottomResultPoint,
				LastLeftBottomPoint);

			// right
			X = Direction.X * UKismetMathLibrary::DegCos(-InAngle) - Direction.Y * UKismetMathLibrary::DegSin(-InAngle);
			Y = Direction.X * UKismetMathLibrary::DegSin(-InAngle) + Direction.Y * UKismetMathLibrary::DegCos(-InAngle);
			FVector RightTopResultPoint = TopStartPoint + FVector(X,Y,0.0f) * Radius;
			FVector RightBottomResultPoint = BottomStartPoint + FVector(X,Y,0.0f) * Radius;

			Draw2DBoxSave(LineData,
				BottomStartPoint,
				TopStartPoint,
				RightTopResultPoint,
				RightBottomResultPoint,
				true);

			// 이전 포인트와 연결 (top)
			DrawLineSave(LineData,
				RightTopResultPoint,
				LastRightTopPoint);

			// 이전 포인트와 연결 (bottom)
			DrawLineSave(LineData,
				RightBottomResultPoint,
				LastRightBottomPoint);

			LastLeftTopPoint = LeftTopResultPoint;
			LastLeftBottomPoint = LeftBottomResultPoint;
			LastRightTopPoint = RightTopResultPoint;
			LastRightBottomPoint = RightBottomResultPoint;
		}
	}
	
	DrawAll(InWorld, LineData, HitColor, CenterPoint, FRotator(RotatePitchValue,0.0f,0.0f), Direction);
}

void UGameDS_DebugManager::Draw2DBoxSave(TArray<FLineDrawSample>& LineData, FVector A, FVector B, FVector C, FVector D, bool bIgnoreABLine)
{
	if (bIgnoreABLine == false)
		DrawLineSave(LineData, A, B);
		
	DrawLineSave(LineData, B, C);
	DrawLineSave(LineData, C, D);
	DrawLineSave(LineData, D, A);
}

void UGameDS_DebugManager::DrawLineSave(TArray<FLineDrawSample>& LineData, FVector A, FVector B)
{
	LineData.Add(FLineDrawSample{A,B});
}

void UGameDS_DebugManager::DrawAll(UWorld* InWorld, TArray<FLineDrawSample>& LineData, FColor InColor, FVector Lot, FRotator Rot, FVector Direction)
{
	for (const FLineDrawSample& Sample : LineData)
	{
		// 회전, 위치 대입
		FVector A = Sample.To.RotateAngleAxis(Rot.Pitch, Direction) + Lot;
		FVector B = Sample.From.RotateAngleAxis(Rot.Pitch, Direction) + Lot;

		DrawDebugLine(InWorld, A, B, InColor, false, CollisionDuration, 0, 2.0f);
	}
}

void UGameDS_DebugManager::Draw2DBox(UWorld* InWorld, FVector A, FVector B, FVector C, FVector D, FColor InColor, bool bIgnoreABLine)
{
	DrawDebugLine(InWorld, A, B, InColor, false, CollisionDuration, 0, 2.0f);
	DrawDebugLine(InWorld, B, C, InColor, false, CollisionDuration, 0, 2.0f);
	DrawDebugLine(InWorld, C, D, InColor, false, CollisionDuration, 0, 2.0f);
	DrawDebugLine(InWorld, D, A, InColor, false, CollisionDuration, 0, 2.0f);
}
