#pragma once
#include "../Unit/GameDS_UnitDefine.h"
#include "../Skill/GameDS_SkillDefine.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameDS_CustomDatas.generated.h"

class AGameDS_Character;
class AGameDS_Summon;
enum class EGameDS_HeroActionState : uint8;

USTRUCT()
struct FGameDS_SpawnData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EGameDS_UnitType UnitType;
	UPROPERTY(EditAnywhere)
	EGameDS_HeroClassType HeroClassType;

	UPROPERTY(EditAnywhere)
	int32 DataID = INDEX_NONE;
	
	UPROPERTY(EditAnywhere, meta=(DisplayThumbnail=true))
	TSubclassOf<AGameDS_Character> CharacterBP;
};

UCLASS(Blueprintable, BlueprintType)
class UGameDS_SpawnUnitData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FGameDS_SpawnData> SpawnDataList;
};

UCLASS(Blueprintable, BlueprintType)
class UGameDS_HeroStatConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_UnitStatInfo UnitStatInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_HeroStatInfo HeroStatInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_HeroCreateInfo HeroCreateInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EGameDS_HeroActionState, float> RequiredActionStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestoredStaminaLockDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustionDuration = 0.0f;
};

USTRUCT(BlueprintType)
struct FGameDS_EnemyBattleModeSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChangeModeDelay = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D BattleModeRange;
};

UENUM(BlueprintType)
enum class EGameDS_EnemyBattleModeType : uint8
{
	None = 0,
	NoneBattleMode,
	NormalAttackMode,
	RangedAttackMode,
	BossSecondPhaseMode,
	Num
};

USTRUCT(BlueprintType)
struct FGameDS_EnemyStatConfigDataTable : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_UnitStatInfo UnitStatInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicalAttackValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicAttackValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed = 170.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FaceOffSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PatrolSpeed = 170.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaximumChasableDistance = 2500.0f;

	// Battle Mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_EnemyBattleModeType DefaultBattleMode = EGameDS_EnemyBattleModeType::NormalAttackMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EGameDS_EnemyBattleModeType, FGameDS_EnemyBattleModeSetting> BattleModeSettingMap;
};

USTRUCT(BlueprintType)
struct FGameDS_SkillDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayThumbnail=true))
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_SkillType SkillType = EGameDS_SkillType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalPercentage = 0.0f;
};

USTRUCT(BlueprintType)
struct FGameDS_SkillInfoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SkillInfoName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollisionInfoID = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> CrowdControlInfoIDList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageValue = 0.0f;
};

/*
** FGameDS_CollisionInfoDataTable 도형별 파라미터 **
@ Box
	Param1 = AttackRange
	Param2 = AttackWidth
	Param3 = AttackHeight

@ Sphere
	Param1 = AttackRange
	Param2 = AttackRadius

@ Capsule
	Param1 = AttackRange
	Param2 = AttackRadius
*/
USTRUCT(BlueprintType)
struct FGameDS_CollisionInfoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollisionInfoID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_CollisionShape CollisionShape = EGameDS_CollisionShape::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_SkillSpawnType CollisionSpawnType = EGameDS_SkillSpawnType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Param0 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Param1 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Param2 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Param3 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Param4 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Param5 = 0.0f;
};

UENUM(BlueprintType)
enum class EGameDS_EnemySkillType : uint8
{
	None = 0,
	Normal,
	ForwardSpecial,
	BackwardSpecial,
	FaceOffSpecial,
	Evade,
	ModeTransition,
	LongDistanceSpecial,
	Num
};

USTRUCT(BlueprintType)
struct FGameDS_EnemySkillSettingDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_EnemySkillType SkillType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_EnemyBattleModeType BattleType = EGameDS_EnemyBattleModeType::NormalAttackMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaitCount = 0; 

	// 스킬 사용 가능 최소 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 0.0f;
	// 스킬 사용 가능 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinAngleDifference = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeedFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveBlockDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackBlockDuration = 1.0f;
};

UENUM(BlueprintType)
enum class EGameDS_ItemType : uint8
{
	None,
	Potion,
	Weapon
};

USTRUCT(BlueprintType)
struct FGameDS_ItemDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ItemImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGameDS_ItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemDescription;
};

USTRUCT(BlueprintType)
struct FGameDS_SkillSetDataTable : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SkillSetName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> SkillSet;
};

UENUM(BlueprintType)
enum class EGameDS_SummonType : uint8
{
	None,
	Default,
	Follower
};

USTRUCT(BlueprintType)
struct FGameDS_SummonDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SummonName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_SummonType SummonType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_SkillSpawnType SpawnType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillInfoID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SummonSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SummonDuration = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SummonMaxDistance = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SummonInitialDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SummonMaxAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SummonDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCollisionDisappearance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyDespawnByDuration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayThumbnail=true))
	TSubclassOf<AGameDS_Summon> SummonBP = nullptr;
};

UENUM(BlueprintType)
enum class EGameDS_GripType : uint8
{
	None,
	OneHand,
	TwoHand
};

UENUM(BlueprintType)
enum class EGameDS_WeaponType : uint8
{
	None,
	Sword,
	LongSword,
	Axe,
	Shield,
};

UENUM(BlueprintType)
enum class EGameDS_WeaponSpawnType : uint8
{
	None,
	LeftWeapon,
	RightWeapon,
	LeftShield,
	RightShield
};

USTRUCT(BlueprintType)
struct FGameDS_WeaponDataTable : public FGameDS_ItemDataTable
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_GripType GripType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_WeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_WeaponSpawnType SpawnType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayThumbnail = true))
	TMap<FString,UAnimMontage*> Montages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredVaitality = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredEndurance = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredStrength = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredIntelligence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicalDamage = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicalGuard = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicGuard = 0.0f;
};

USTRUCT(BlueprintType)
struct FGameDS_PotionDataTable : public FGameDS_ItemDataTable
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DefaultUseCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultHealValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 5.0f;
};

USTRUCT(BlueprintType)
struct FGameDS_HeroSkillSetDataTable : public FGameDS_SkillSetDataTable
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_WeaponType WeaponType;
};

UENUM(BlueprintType)
enum class EGameDS_CrowdControlType : uint8
{
	None = 0,
	PushBack,
	NormalKnockBack,
	HeavyKnockBack,
	Stun,
	Num
};

UENUM(BlueprintType)
enum class EGameDS_CrowdControlMoveType : uint8
{
	None = 0,
	CasterToTarget,
	TargetToCaster,
	SummonToTarget,
	SummonToCaster,
	Num
};

USTRUCT(BlueprintType)
struct FGameDS_CrowdControlData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_CrowdControlType CrowdControlType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_CrowdControlMoveType CrowdControlMoveType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveBlockDuration = 0.0f;
};

USTRUCT(BlueprintType)
struct FGameDS_CrowdControlInfoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrowdControlName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_CrowdControlData CrowdControlData;
};

UENUM(BlueprintType)
enum class EGameDS_StatusEffectType : uint8
{
	None = 0,
	Invincibility,
	Num
};

USTRUCT(BlueprintType)
struct FGameDS_StatusEffectInfoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatusEffectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_StatusEffectType StatusEffectType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDuration = INDEX_NONE;
};

UCLASS(Blueprintable, BlueprintType)
class UGameDS_DefaultHeroSetting : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(Category="Sword", EditAnywhere, BlueprintReadWrite)
	int32 SwordLHandWeaponDataID = INDEX_NONE;

	UPROPERTY(Category="Sword", EditAnywhere, BlueprintReadWrite)
	int32 SwordRHandWeaponDataID = INDEX_NONE;

	UPROPERTY(Category="Axe", EditAnywhere, BlueprintReadWrite)
	int32 AxeLHandWeaponDataID = INDEX_NONE;

	UPROPERTY(Category="Axe", EditAnywhere, BlueprintReadWrite)
	int32 AxeRHandWeaponDataID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PotionDataID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> InventoryItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_HeroStatInfo HeroStatInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameDS_UnitStatInfo UnitStatInfo;
};
