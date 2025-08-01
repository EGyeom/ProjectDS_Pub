#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameDS_UnitDefine.h"
#include "GameDS_Character.generated.h"

class USoundCue;
enum class EGameDS_DamageType : uint8;
class UGameDS_CrowdControlController;
class UGameDS_BattleSubSystem;
class UGameDS_StatController;
class UGameDS_StatusEffectController;
class UWidgetComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FCharacterOnDamageDelegate, const FGameDS_DamageInfo&)
DECLARE_MULTICAST_DELEGATE(FCharacterOnDeadDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FCharacterStatUpdateDelegate, UGameDS_StatController*)

UCLASS()
class PROJECTDS_API AGameDS_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AGameDS_Character();
	
	FCharacterOnDamageDelegate OnDamageDelegate;
	FCharacterOnDeadDelegate OnDeadDelegate;
	
	FCharacterStatUpdateDelegate StatUpdateDelegate;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo, const FGameDS_HeroStatInfo& InHeroStatInfo);
	virtual EGameDS_UnitType GetUnitType() const { return UnitType; }
	virtual bool IsUnitType(EGameDS_UnitType InUnitType);

	virtual void OnDamaged(const FGameDS_DamageInfo& InDamageInfo);
	virtual void OnReset();
	virtual void OnDead();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDamageWithBP(FVector InLocation, FVector InImpactNormal);

	void UpdateStat();
	
	int32 GetUnitID() const { return UnitID; }
	int32 GetDataID() const { return DataID; }

	UGameDS_StatController* GetStatController() const { return StatController; }
	UGameDS_CrowdControlController* GetCrowdControlController() const { return CrowdController; }
	UGameDS_StatusEffectController* GetStatusEffectController() const { return StatusEffectController; }

	bool IsDied() const { return bDied; }
	void SetDie(bool InDie) { bDied = InDie; }
	
	FName GetLeftFootSocketName() const { return LeftFootSocket; }
	FName GetRightFootSocketName() const { return RightFootSocket; }
	FName GetRootSocketName() const { return RootSocket; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitWaterSimWithBP(UObject* WaterSimBP);

	UFUNCTION(BlueprintCallable)
	void PlayCCMotion(EGameDS_CrowdControlType InCCType);
	
protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	USoundCue* DamagedSound;

private:
	UPROPERTY()
	UGameDS_StatController* StatController;
	UPROPERTY()
	UGameDS_CrowdControlController* CrowdController;
	UPROPERTY()
	UGameDS_StatusEffectController* StatusEffectController;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	FName LeftFootSocket = TEXT("Foot_L");
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	FName RightFootSocket = TEXT("Foot_R");
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	FName RootSocket = TEXT("Root");

	bool bDied = false;
	
	EGameDS_UnitType UnitType = EGameDS_UnitType::None;
	int32 UnitID = INDEX_NONE;
	int32 DataID = INDEX_NONE;
};

template <typename T>
T* GameDS_CastCharacter(AGameDS_Character* Character)
{
	if (Character == nullptr)
		return nullptr;
	
	if (Character->GetUnitType() != EGameDS_UnitType::None)
		return Cast<T>(Character);

	return nullptr;
}