#pragma once
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "GameDS_StatController.generated.h"

class AGameDS_Character;
class UGameDS_HeroStatConfigData;

UENUM(BlueprintType)
enum class EGameDS_HeroActionState : uint8
{
	None = 0,
	Attack,
	Evade,
	Run,
	Defense,
	StrongAttack
};

enum class EGameDS_StaminaRestoreType : uint8
{
	None = 0,
	Restore,
	Delay,
	Exhaustion
};

UCLASS()
class UGameDS_StatController : public UObject
{
	GENERATED_BODY()

public:
	UGameDS_StatController();

	void Tick(float DeltaTime, TArray<EGameDS_StatResult>& StatResultList);

	void Init(const FGameDS_UnitStatInfo& LastStat);
	void Init(const FGameDS_HeroStatInfo& LastStat, AGameDS_Character* InCharacter);

	void LoadStat(const FGameDS_HeroCreateInfo& InHeroCreateInfo);
	void SaveStat();

	float GetStatValue(const EGameDS_StatType StatType) const;
	float GetHealthPercentage() const;
	float GetStaminaPercentage() const;

	const FGameDS_UnitStatInfo& GetStatInfoConfig() const { return StatInfoConfig; }
	const FGameDS_UnitStatInfo& GetStatInfo() const { return StatInfo; }
	const FGameDS_HeroStatInfo& GetHeroStatInfo() const { return HeroStatInfo; }

	EGameDS_StaminaRestoreType GetStaminaRestoreType() const { return StaminaRestoreType; }

	void OnDamaged(float InDamageValue, TArray<EGameDS_StatResult>& StatResultList);
	void OnHealed(float InHealValue, TArray<EGameDS_StatResult>& StatResultList);
	void SetAction(EGameDS_HeroActionState InState);
	bool OnUseStamina(float InRequiredStamina, TArray<EGameDS_StatResult>& StatResultList);
	bool OnUseStamina(EGameDS_HeroActionState InState, TArray<EGameDS_StatResult>& StatResultList);
	void OnResetStatCondition();
	bool IsMale() const;

private:
	void UpdateStamina(float DeltaTime);
	void RestoreStamina(float DeltaTime, float Speed = 1.0f);

	EGameDS_GenderType GenderType;
	
	// 스텟 최대치
	FGameDS_UnitStatInfo StatInfoConfig;
	// 현재 스텟
	FGameDS_UnitStatInfo StatInfo;
	FGameDS_HeroStatInfo HeroStatInfo;
	EGameDS_HeroActionState HeroActionState;

	EGameDS_StaminaRestoreType StaminaRestoreType;

	float ExhaustionRemainTime = 0.0f;
	float RestoredStaminaLockRemainTime = 0.0f;

	UPROPERTY()
	AGameDS_Character* OwnerCharacter;

	UPROPERTY()
	const UGameDS_HeroStatConfigData* HeroStatConfigData;
};
