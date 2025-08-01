#pragma once
#include "ProjectDS/Skill/GameDS_SkillDefine.h"
#include "GameDS_UnitDefine.generated.h"

struct FGameDS_SummonDataTable;
enum class EGameDS_DamageType : uint8;

UENUM(BlueprintType)
enum class EGameDS_UnitType : uint8
{
	None = 0,
	Hero,
	UserHero,
	Enemy,
	Boss,
	SummonObject
};

UENUM(BlueprintType)
enum class EGameDS_HeroClassType : uint8
{
	None = 0,
	Warrior,
	Magician
};

UENUM(BlueprintType)
enum class EGameDS_HeroAnimNotifyState : uint8
{
	Moveable = 0,
	Attackable,
	Attacking
};

UENUM(BlueprintType)
enum class EGameDS_HeroActionType : uint8
{
	None = 0,
	Look,
	Move,
	Jump,
	NormalAttack,
	SpecialAttack,
	Evade,
	Defense,
	Menu,
	Inventory,
	Status,
	ChangeItem,
	DrinkPotion,
	Interaction
};

enum class EGameDS_StatType : uint8
{
	None = 0,
	Health,
	Stamina,
	MaximumHealth,
	MaximumStamina
};

enum class EGameDS_StatResult
{
	None = 0,
	ZeroHealth,
	ZeroStamina,
	FullHealth,
	FullStamina
};

UENUM(BlueprintType)
enum class EGameDS_CreateWeaponType : uint8
{
	None,
	Axe,
	Sword
};

UENUM(BlueprintType)
enum class EGameDS_GenderType : uint8
{
	Male,
	Female
};

USTRUCT(BlueprintType)
struct FGameDS_UnitStatInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Focus = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stamina = INDEX_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PhysicalDefence = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagicDefence = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FGameDS_HeroStatInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VitalityPoint = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndurancePoint = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StrengthPoint = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IntelligencePoint = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FGameDS_HeroCreateInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_GenderType GenderValue = EGameDS_GenderType::Male;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_CreateWeaponType Weapon = EGameDS_CreateWeaponType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnPoint = 0;

	int32 LHandIndex = 0;
	int32 RHandIndex = 0;
	int32 PotionIndex = 0;

	TArray<FString> LHandList;
	TArray<FString> RHandList;
	TArray<FString> PotionList;
	TArray<FString> InventoryItemList;

	TArray<FString> HeroStatList;
	TArray<FString> UnitStatList;
};

USTRUCT(BlueprintType)
struct FGameDS_UnitSpawnOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameDS_UnitType UnitType = EGameDS_UnitType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitID = INDEX_NONE;
	int32 DataID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnPoint = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotator = FRotator::ZeroRotator;

	bool bBattleMode = false;
	bool bUsePatrol = false;
	TArray<FVector> SplinePathPoint;
};

struct FGameDS_SummonUnitSpawnOption
{
	int32 OwnerUnitID = INDEX_NONE;
	int32 TargetUnitID = INDEX_NONE;
	int32 SummonUnitID = INDEX_NONE;
	int32 SummonDataID = INDEX_NONE;

	FVector Location = FVector::ZeroVector;
	FRotator Rotator = FRotator::ZeroRotator;
};

struct FGameDS_DamageInfo
{
	int32 AttackedUnitID = INDEX_NONE;
	int32 AttackedSummonUnitID = INDEX_NONE;
	float DamageValue = 0;
	EGameDS_DamageType DamageType;
	FVector HitLocation = FVector::ZeroVector;
	FVector HitNormal = FVector::ZeroVector;
};

UENUM(BlueprintType)
enum class EGameDS_UnitAnimType : uint8
{
	None = 0,
	NormalAttack,
	SpecialAttack,
	Dash,
	Die,
	ModeTransition,
	Num
};

UENUM(BlueprintType)
enum class EGameDS_BlockType : uint8
{
	None=0,
	MoveBlock,
	ActionBlock,
	RotateBlock
};

UENUM(BlueprintType)
enum class EGameDS_ArrowKey : uint8
{
	Left = 0,
	Right,
	Up,
	Down
};

UENUM(BlueprintType)
enum class EGameDS_FootStepType : uint8
{
	None=0,
	Left,
	Right,
	Both
};