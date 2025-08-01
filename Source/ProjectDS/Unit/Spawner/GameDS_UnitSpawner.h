#pragma once
#include "GameFramework/Actor.h"
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "GameDS_UnitSpawner.generated.h"

class USplineComponent;
UCLASS()
class AGameDS_UnitSpawner : public AActor
{
	GENERATED_BODY()
	virtual void BeginPlay() override;
protected:	
	AGameDS_UnitSpawner();
	virtual void PostInitializeComponents() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameDS_UnitType UnitType = EGameDS_UnitType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DataID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	bool bUsePatrol = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	bool bBattleMode = false;

	UFUNCTION(CallInEditor, Category = "Patrol", meta = (AllowPrivateAccess = "true"))
	void CallRetargetPathAndDraw();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	USplineComponent* PatrolSplinePath;
};
