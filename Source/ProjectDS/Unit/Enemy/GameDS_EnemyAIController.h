#pragma once
#include "AIController.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "GameDS_EnemyAIController.generated.h"

enum class EGameDS_UnitAnimType : uint8;
class AGameDS_HeroCharacter;
class AGameDS_Enemy;
class UGameDS_UnitAnimInstance;
struct FGameDS_EnemyStatConfigDataTable;

UENUM(BlueprintType)
enum class EGameDS_EnemyAIMode : uint8
{
	None = 0,
	Idle,
	Patrol,
	Chase,
	Attack,
	AttackRest,
	FaceOff,
	AttackSelection,
	Hit,
	Die,
	Evade,
	ModeTransition,
	PhaseEnd,
	Num
};

struct FGameDS_EnemyDetailSkillData
{
	int32 WaitCount;
	int32 MaxWaitCount;
	
	bool bMontageSkill;
	int32 SkillID = INDEX_NONE;
};

struct FGameDS_EnemySkillOrderData
{
	int32 OrderIndex = 0;
	TArray<FGameDS_EnemyDetailSkillData> DetailSkillDataList;
};

struct FGameDS_EnemyBattleModeOrderData
{
	TMap<EGameDS_EnemySkillType, FGameDS_EnemySkillOrderData> CombatOrder;
	int32 SavedSkillID = INDEX_NONE;
};

UCLASS()
class AGameDS_EnemyAIController : public AAIController 
{
	GENERATED_BODY()

public:
	virtual void Init(AGameDS_Enemy* InEnemy, UGameDS_UnitAnimInstance* InAnimInstance);
	virtual void LastInit();
	virtual void InitPatrolPath(const TArray<FVector>& SplinePath);
	virtual void Tick(float DeltaSeconds) override;
	virtual void UpdateAction(float DeltaSeconds);
	virtual void UpdateRotateValue(float DeltaSeconds);

	virtual void ChangeBattleModeSetting(bool bActive, AGameDS_HeroCharacter* InTarget = nullptr);
	virtual void ChangeBattleModeType(EGameDS_EnemyBattleModeType InBattleMode);

	void DestroyWithDelay(float InDelay);
	
	void ChangeBattleModeTypeWithDistance();
	bool CheckBattleModeChangeable(EGameDS_EnemyBattleModeType& ChangeableMode);

	UFUNCTION(BlueprintCallable)
	void ChangeRotateSetting(bool bInSetting);
	
	UFUNCTION(BlueprintCallable)
	bool CheckAttackRange();

	UFUNCTION(BlueprintCallable)
	bool CheckAttackAngle();

	UFUNCTION(BlueprintCallable)
	bool CheckFaceOffAngle();

	UFUNCTION(BlueprintCallable)
	void UpdateState(EGameDS_EnemyAIMode InAIMode);

	UFUNCTION(BlueprintCallable)
	FVector NextPatrolPoint();
	UFUNCTION(BlueprintCallable)
	FVector GetPatrolPoint();

	UFUNCTION(BlueprintCallable)
	bool RandomEvade();

	EGameDS_EnemyAIMode AIMode = EGameDS_EnemyAIMode::None;
	EGameDS_EnemyAIMode BeforeHitAIMode = EGameDS_EnemyAIMode::None;
	
	UPROPERTY()
	AGameDS_Enemy* OwnerEnemy;
	UPROPERTY()
	AGameDS_HeroCharacter* Target;
	UPROPERTY()
	UGameDS_UnitAnimInstance* AnimInstance;

	FSimpleDelegate ResetTargetDelegate;

	UFUNCTION(BlueprintCallable)
	void CalculrateTargetLocation(bool ToCharacter = true, float InDistance = 100.f, float InGap = 100.f);

	UFUNCTION(BlueprintCallable)
	void SetTargetLocation(bool InSetted, float InFollowSpeed, bool InLastMove = true);

	void CachingSkillSetting();

protected:
	float GetCalculateRotateValue();
	EGameDS_EnemySkillType GetCulNextSkillType();
	TArray<FGameDS_EnemyDetailSkillData>& GetDetailSkillList(EGameDS_EnemyBattleModeType InBattleModeType, EGameDS_EnemySkillType InSkillType);
	FGameDS_EnemyDetailSkillData* GetFrontSkillData(EGameDS_EnemySkillType InSkillType);
	void NextOrder(EGameDS_EnemySkillType InSkillType);
	void FollowTarget(float DeltaTime);
	void SetFaceOffMode(bool bEnable);
	void SaveFaceOffDistance();
	float GetDistanceToTarget() const;
	bool IsInRangeChasableDistance() const;
	
	void CheckSprintCondition();

	EGameDS_EnemySkillType NextSkillType;
	int32 NextSkillID = INDEX_NONE;
	
	float RestElapsedTime = 0.0f;
	float RestDuration = 0.0f;
	bool bRotate = false;
	bool bHit = false;
	bool bPlayModeTransitionAnim = false;
	FVector SavedTargetLocation;
	bool bPatrolUnit = false;
	int32 NextPatrolIndex = 1;
	bool bSetTargetLocation = false;
	
	float AttackDuration = 0.f;
	float FollowSpeed = 0.f;
	TArray<FVector> PatrolPoint;
	TMap<int32, const FGameDS_EnemySkillSettingDataTable> SkillSettingMap;
	const FGameDS_EnemyStatConfigDataTable* StatConfigData;
	
	TMap<EGameDS_EnemyBattleModeType, FGameDS_EnemyBattleModeOrderData> CombatOrderMap;
	EGameDS_EnemyBattleModeType BattleMode;
	EGameDS_EnemyBattleModeType DefaultBattleMode;

	float BeginFaceOffDistance = 0.0f;
	float ModeChangeDelayElapsedTime = 0.0f;

	float DestroyRemainTime = 0.0f;
	bool bActiveDestroy = false;
	bool bActiveRotate = false;
	float RotateToTargetAngle = -1.0f;

	bool bLastMoving = false;
};