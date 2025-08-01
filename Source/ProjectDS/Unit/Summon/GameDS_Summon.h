#pragma once
#include "ProjectDS/Manager/GameDS_BattleSubSystem.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "GameDS_Summon.generated.h"

class AGameDS_Character;
class USoundCue;

DECLARE_DELEGATE_OneParam(FGameDS_SummonDespawnDelegate, AGameDS_Summon*);
DECLARE_DELEGATE_RetVal_FourParams(bool, FGameDS_SummonCollisionCheckDelegate, const FGameDS_DetailCollisionInfo&, FVector, FVector, bool)

UCLASS()
class AGameDS_Summon : public AActor
{
	GENERATED_BODY()

public:
	AGameDS_Summon();
	
	virtual void Tick(float DeltaSeconds) override;
	
	void Init(AGameDS_Character* InOwner, AGameDS_Character* InTarget, const FGameDS_DetailCollisionInfo& InDetailCollisionInfo, const FGameDS_SummonDataTable* InSummonData);
	void UpdateDuration(float DeltaTime);
	void CheckDistance();
	void CheckDelay(float DeltaTime);
	int32 GetSummonOwnerUnitID() const { return SummonOwnerUnitID; }
	
	UFUNCTION(BlueprintCallable)
	AGameDS_Character* GetTargetCharacter() { return Target; }

	UFUNCTION(BlueprintCallable)
	int32 GetSkillID() { return SkillID; }

	UFUNCTION(BlueprintCallable)
	bool GetSoundOn() { return bSoundOn; }

	UFUNCTION(BlueprintCallable)
	USoundCue* GetSoundCue() { return SoundCue; }

	UFUNCTION(BlueprintNativeEvent)
	void OnDespawn();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnSystemFinished();

	FGameDS_SummonDespawnDelegate SummonDespawnDelegate;
	FGameDS_SummonCollisionCheckDelegate SummonCollisionCheckDelegate;

private:
	int32 SummonOwnerUnitID = INDEX_NONE;
	int32 SummonUnitID = INDEX_NONE;
	
	UPROPERTY()
	AGameDS_Character* SummonOwner;

	UPROPERTY()
	AGameDS_Character* Target;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	USoundCue* SoundCue;

	FGameDS_DetailCollisionInfo DetailCollisionInfo;
	FVector InitialLocation;

	EGameDS_SummonType SummonType = EGameDS_SummonType::None;
	EGameDS_DamageType DamageType = EGameDS_DamageType::None;
	
	float Damage = 0.0f;
	float SummonSpeed = 0.0f;
	float SummonElapsedTime = 0.0f;
	float SummonDuration = 0.0f;
	float SummonMaxDistance = 0.0f;
	float SummonDelay = 0.0f;
	bool bActive = false;
	bool bAfterDelay = false;
	bool bCollisionDisappearance = false;
	bool bOnlyDespawnByDuration = false;
	bool bHit = false;
	int32 SkillID = 0;
	bool bSoundOn = false;
	float SoundCheckTimer = 0.f;
};
