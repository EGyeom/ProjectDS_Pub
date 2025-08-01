#pragma once
#include "ProjectDS/Manager/Object/GameDS_ManagerObject.h"
#include "GameDS_DebugManager.generated.h"

enum class EGameDS_CollisionShape : uint8;
class AGameDS_Character;
class UGameDS_UnitInfoWidget;
struct FGameDS_CollisionInfoDataTable;

struct FLineDrawSample
{
	FVector From;
	FVector To;
};

UCLASS()
class UGameDS_DebugManager : public UGameDS_ManagerObject
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime, UGameInstance* InGameInstance) override;

	void SetDebugMode(UGameInstance* InGameInstance);
	void InitDebugSetting(UGameInstance* InGameInstance);
	void SetDebugDistance(float InDistanceValue);
	void SetCollisionDuration(float InCollisionDuration);

	void MakeDebugCollision(AGameDS_Character* Owner, const FGameDS_CollisionInfoDataTable* CollisionInfo, bool bSummon, FVector SummonDirection, FVector SummonLocation, bool Hit);
	void MakeDebugCollision(AGameDS_Character* Owner, const FGameDS_CollisionInfoDataTable* CollisionInfo, bool Hit, bool bSummon = false, FVector SummonDirection = FVector::ZeroVector, FVector SummonLocation = FVector::ZeroVector);

private:
	void DrawDebugCylinderWithAngle(UWorld* InWorld, FVector CenterPoint, float HalfHeight, float Radius, float Angle, FVector Direction, float RotatePitchValue, FColor HitColor);
	void Draw2DBoxSave(TArray<FLineDrawSample>& LineData, FVector A, FVector B, FVector C, FVector D, bool bIgnoreABLine);
	void DrawLineSave(TArray<FLineDrawSample>& LineData, FVector A, FVector B);
	void DrawAll(UWorld* InWorld, TArray<FLineDrawSample>& LineData, FColor InColor, FVector Lot, FRotator Rot, FVector Direction);
	void Draw2DBox(UWorld* InWorld, FVector A, FVector B, FVector C, FVector D, FColor InColor, bool bIgnoreABLine);
	
	// UnitInfo
	bool bActiveDebug = false;
	const float MaximumTickDelay = 0.5f;
	float TickDelay = 0.0f;

	// Collision
	float CollisionDuration = 1.5f;

	// 20 meter
	float DebugDistance = 2000.0f;

	UPROPERTY()
	TMap<int32, UGameDS_UnitInfoWidget*> VisibleUnitInfoMap;
	TMap<int32, UGameDS_UnitInfoWidget*> UnVisibleUnitInfoMap;
	
	//TArray<FGameDS_DebugCollisionInfo> DebugCollisionList;
};