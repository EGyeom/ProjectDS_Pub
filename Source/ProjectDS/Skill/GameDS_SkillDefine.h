#pragma once

UENUM(BlueprintType)
enum class EGameDS_SkillType : uint8
{
	None = 0,
	Normal,
	Active,
	Charge
};

UENUM(BlueprintType)
enum class EGameDS_DamageType : uint8
{
	None = 0,
	Light,
	Normal,
	Heavy
};

UENUM(BlueprintType)
enum class EGameDS_CollisionShape : uint8
{
	None = 0,
	Box,
	Sphere,
	Capsule,
	Cylinder
};

UENUM(BlueprintType)
enum class EGameDS_SkillSpawnType : uint8
{
	None = 0,
	CasterForward,
	WeaponSocket,
	Target,
	Self
};