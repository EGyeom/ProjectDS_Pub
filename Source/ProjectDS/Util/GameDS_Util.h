#pragma once
#define COLLISION_CHANNEL ECollisionChannel::ECC_EngineTraceChannel2
#define COLLISION_CHANNEL_ENEMY ECollisionChannel::ECC_GameTraceChannel4
#define COLLISION_CHANNEL_LOCKON ECollisionChannel::ECC_GameTraceChannel3
#define LOCKOFF_SPRINGARM_LENGTH 350.f
#define LOCKON_SPRINGARM_LENGTH 450.f
#define INVENTORY_MINWIDTH 5
#define ALLITEMTYPE_COUNT 4
#define MISCELLANEOUS_MINHEIGHT 2
#define EQUIPPEDWEAPON_MAXCOUNT 5
#define EQUIPPEDPOTION_MAXCOUNT 2
#define PLAYER_RUNSPEED 1000
#define PLAYER_WALKSPEED 600
#define PLAYER_DEFAULT_JUMP_HEIGHT 420
#define ENEMY_HEALTH_VISIBLE_TIME 10.0f

#define METER_TO_CENTIMETER 0.01f
#define CENTIMETER_TO_METER 100.0f
#include "ProjectDS/Unit/GameDS_UnitDefine.h"

struct FGameDS_Util
{
	static FRotator ComposeRotator(FRotator RotA, FRotator RotB)
	{
		return FRotator(FQuat(RotA) * FQuat(RotB));
	}
	static FRotator ComposeRotator(FRotator InRotator, float InAddYaw, float InDeltaTime = 1.0f)
	{
		return FRotator(FQuat(InRotator) * FQuat(FRotator(0.0f, InAddYaw * InDeltaTime, 0.0f)));
	}
	static float GetDifferenceDegree(FVector VectorA, FVector VectorB)
	{
		const float DotResult = FVector::DotProduct(VectorA, VectorB);
		return FMath::RadiansToDegrees(FMath::Acos(DotResult));
	}
	static float GetDifferenceDegree2D(FVector VectorA, FVector VectorB)
	{
		const float DotResult = FVector::DotProduct(VectorA.GetSafeNormal2D(), VectorB.GetSafeNormal2D());
		return FMath::RadiansToDegrees(FMath::Acos(DotResult));
	}

	static void ParsingUnitInfo(const FGameDS_HeroCreateInfo& InHeroCreateInfo, FGameDS_HeroStatInfo& HeroStatInfo, FGameDS_UnitStatInfo& UnitStatInfo);
};
