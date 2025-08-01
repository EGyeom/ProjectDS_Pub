// Fill out your copyright notice in the Description page of Project Settings.

#include "GameDS_BattleSubSystem.h"
#include "GameDS_CustomDatas.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Util/GameDS_Util.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "GameDS_GameManager.h"
#include "GameDS_SummonManager.h"
#include "GameDS_UnitManager.h"
#include "Debug/GameDS_DebugManager.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectDS/Unit/Controller/GameDS_CrowdControlController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatusEffectController.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "ProjectDS/Unit/Summon/GameDS_Summon.h"

class UGameDS_UnitManager;

UGameDS_BattleSubSystem::UGameDS_BattleSubSystem()
{

}

void UGameDS_BattleSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameDS_BattleSubSystem::Deinitialize()
{
	Super::Deinitialize();
}

FGameDS_DetailCollisionInfo UGameDS_BattleSubSystem::MakeSkillCollision(int32 SkillInfoID, AGameDS_Character* OwnerCharacter, bool bSummon, bool bNeutral) const
{
	FGameDS_DetailCollisionInfo DetailCollisionInfo;
	
	if (OwnerCharacter == nullptr)
		return DetailCollisionInfo;
	
	if (UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance()))
	{
		if (const FGameDS_SkillInfoDataTable* CurrentSkillInfoData = DataManager->GetSkillInfoData(SkillInfoID))
		{
			if (const FGameDS_CollisionInfoDataTable* CurrentCollisionInfoData = DataManager->GetCollisionInfoData(CurrentSkillInfoData->CollisionInfoID))
			{
				const bool bHeroSkill = OwnerCharacter->IsUnitType(EGameDS_UnitType::Hero);

				DetailCollisionInfo.CollisionInfoData = CurrentCollisionInfoData;
				DetailCollisionInfo.CollisionShape = MakeCollisionShape(CurrentCollisionInfoData);
				DetailCollisionInfo.OwnerCharacter = OwnerCharacter;
				DetailCollisionInfo.DamageValue = CurrentSkillInfoData->DamageValue;
				DetailCollisionInfo.bHeroSkill = bHeroSkill;
				DetailCollisionInfo.bNeutral = bNeutral;
				DetailCollisionInfo.bSummon = bSummon;
				DetailCollisionInfo.bValidData = true;
				DetailCollisionInfo.CrowdControlIDList = CurrentSkillInfoData->CrowdControlInfoIDList;

				return DetailCollisionInfo;
			}
		}
	}

	return DetailCollisionInfo;
}

bool UGameDS_BattleSubSystem::OnAttackCheck(const FGameDS_DetailCollisionInfo& DetailCollisionInfo, FVector NormalDirection, FVector Location, bool bHitCharacter)
{
	FCollisionQueryParams CollisionParams(NAME_None, false, DetailCollisionInfo.OwnerCharacter);
	FCollisionShape CollisionShape = DetailCollisionInfo.CollisionShape;
	FVector StartLocation;
	FVector AttackDirection;
	ECollisionChannel CollisionChannel = ECC_Visibility;
	const FGameDS_CollisionInfoDataTable& CollisionInfo = *DetailCollisionInfo.CollisionInfoData;

	bool bHitTarget = false;
	bool bHitSomething = false;
	bool bEvading = false;
	/*
	** FGameDS_CollisionInfoDataTable 도형별 파라미터 **
	@ Box
		Param0 = Forward Distance
		Param1 = AttackRange
		Param2 = AttackWidth
		Param3 = AttackHeight

	@ Sphere
		Param0 = Forward Distance
		Param1 = AttackRange
		Param2 = AttackRadius

	@ Capsule
		Param0 = Forward Distance
		Param1 = AttackRange
		Param2 = AttackRadius

	@ Cylinder
		Param0 = Forward Distance
		Param1 = height
		Param2 = radius
		param3 = angle
		param4 = rotate
	*/

	if (DetailCollisionInfo.bSummon == false)
	{
		AttackDirection = DetailCollisionInfo.OwnerCharacter->GetActorForwardVector();
		StartLocation = DetailCollisionInfo.OwnerCharacter->GetActorLocation();

		switch (CollisionInfo.CollisionSpawnType)
		{
		case EGameDS_SkillSpawnType::CasterForward:
			{
				StartLocation += AttackDirection * CollisionInfo.Param0;
			}
			break;
		case EGameDS_SkillSpawnType::WeaponSocket:
			{
				const AGameDS_HeroCharacter* HeroCharacter = Cast<AGameDS_HeroCharacter>(DetailCollisionInfo.OwnerCharacter);
				if (HeroCharacter)
				{
					StartLocation = DetailCollisionInfo.OwnerCharacter->GetMesh()->GetSocketLocation(HeroCharacter->GetWeaponSocketName());
				}
			}
			break;
		case EGameDS_SkillSpawnType::Self:
			{
			}
			break;
		default:
			break;
		}
	}
	else
	{
		AttackDirection = NormalDirection;
		StartLocation = Location;
	}
	
	if (CollisionInfo.CollisionShape != EGameDS_CollisionShape::Cylinder)
	{
		TArray<FHitResult> HitResultList;
		/*
			Box,
			Sphere,
			Capsule,
			Cylinder
		*/
		FVector EndFixPos = StartLocation;
		if (CollisionInfo.CollisionShape == EGameDS_CollisionShape::Capsule)
		{
			StartLocation = StartLocation + FVector(0.0f, 0.0f, CollisionInfo.Param1 * 0.5f);
			EndFixPos = StartLocation - FVector(0.0f, 0.0f, CollisionInfo.Param1 * 0.5f);
		}
		else
		{
			EndFixPos.Z -= 0.001f;
		}

		FRotator Rot = NormalDirection.ToOrientationRotator();
		
		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResultList,
			StartLocation,
			EndFixPos,
			FQuat(Rot),
			CollisionChannel,
			CollisionShape,
			CollisionParams
		);

		for (const FHitResult& Hit : HitResultList)
		{
			// agro sphere일 경우
			if (Hit.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel8)
				continue;

			// 특정 벽에 가로막힐 때
			bHitSomething = true;

			AGameDS_Character* DamagedCharacter = Cast<AGameDS_Character>(Hit.GetActor());
			
			if (DamagedCharacter == nullptr)
				continue;
		
			if (DetailCollisionInfo.bHeroSkill)
			{
				if (DamagedCharacter->IsUnitType(EGameDS_UnitType::Enemy) == false)
					continue;
			}
			else
			{
				if (DetailCollisionInfo.bNeutral == false)
				{
					if (DamagedCharacter->IsUnitType(EGameDS_UnitType::Hero) == false)
						continue;
				}
			}
			
			bHitTarget = true;

			if(AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(DamagedCharacter))
			{
				if(Hero->GetEvading() == true)
				{
					bHitTarget = false;
					bHitSomething = false;
					continue;
				}
			}

			if (DamagedCharacter->GetCrowdControlController())
				DamagedCharacter->GetCrowdControlController()->ApplyCrowdControl(DetailCollisionInfo.CrowdControlIDList, DetailCollisionInfo.OwnerCharacter, StartLocation);

			const float DamageValue = DetailCollisionInfo.DamageValue; 
			UE_LOG(LogTemp, Log, TEXT("%s에게 %f의 데미지를 입혔습니다!"), *DamagedCharacter->GetName(), DamageValue);

			if (DamagedCharacter->OnDamageDelegate.IsBound())
			{
				FGameDS_DamageInfo DamageInfo;
				DamageInfo.DamageType = DetailCollisionInfo.DamageType;
				DamageInfo.DamageValue = DetailCollisionInfo.DamageValue;
				DamageInfo.AttackedUnitID = DetailCollisionInfo.OwnerCharacter->GetUnitID();
				DamageInfo.AttackedSummonUnitID = INDEX_NONE;
				DamageInfo.HitLocation = Hit.ImpactPoint;
				DamageInfo.HitNormal = Hit.ImpactNormal;
				
				if (DamagedCharacter != nullptr)
				{
					UGameDS_StatusEffectController* StatusEffectController = DamagedCharacter->GetStatusEffectController();
					if (StatusEffectController == nullptr)
						continue;

					if (StatusEffectController->IsDamageable() == false)
						continue;
					
					if (bHitCharacter == false)
						StatusEffectController->AddStatusEffect(102);
				}
				
				DamagedCharacter->OnDamageDelegate.Broadcast(DamageInfo);
			}
		}
	}
	else
	{
		/*
		@ Cylinder
		Param1 = height
		Param2 = radius
		param3 = angle
		param4 = rotate
		 */
		
		// cylinder
		FVector UpVec =  FVector(0.0f, 0.0f, CollisionInfo.Param1 * 0.5f);
		UpVec = UpVec.RotateAngleAxis(CollisionInfo.Param4, AttackDirection);
		
		FVector StartTopLocation = StartLocation + UpVec;
		FVector StartBottomLocation = StartLocation - UpVec;
		FRotator StartRot;
		StartRot.Pitch = CollisionInfo.Param4;
		
		TArray<FHitResult> HitResultList;
		bool bHit = GetWorld()->SweepMultiByChannel(
			HitResultList,
			StartTopLocation,
			StartBottomLocation,
			FQuat(),
			CollisionChannel,
			CollisionShape,
			CollisionParams
		);

		for (const FHitResult& Hit : HitResultList)
		{
			// agro sphere일 경우
			if (Hit.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel8)
				continue;

			AGameDS_Character* DamagedCharacter = Cast<AGameDS_Character>(Hit.GetActor());
			if (DamagedCharacter != nullptr)
			{
				if (AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(DamagedCharacter))
				{
					if (Hero->GetEvading())
						continue;
				}
			}
			
			bHitSomething = true;
			
			//1. 각도 검사
			FVector NormalV = (Hit.ImpactPoint - StartLocation).GetSafeNormal2D();
			float DotResult = FVector::DotProduct(NormalV, AttackDirection.GetSafeNormal2D());
			float Angle = FMath::RadiansToDegrees(FMath::Acos(DotResult));

			if (Angle > CollisionInfo.Param3)
				continue;

			FVector CollisionNormal = UpVec.GetSafeNormal();
				
			//2. 캡슐 상단 검사
			FVector CapsuleTopSphereLocation = StartLocation;
			CapsuleTopSphereLocation += CollisionNormal * CollisionInfo.Param1 * 0.5f;
				
			FVector ImpactToTopSphere = (Hit.ImpactPoint - CapsuleTopSphereLocation).GetSafeNormal();
			float Deg1 = FMath::RadiansToDegrees(FMath::Acos(CollisionNormal | ImpactToTopSphere));

			if (Deg1 <= 90.0f)
				continue;
			
			//3. 캡슐 하단 검사
			FVector CapsuleBottomSphereLocation = StartLocation;
			CapsuleBottomSphereLocation += -CollisionNormal * CollisionInfo.Param1 * 0.5f;

			FVector ImpactToBottomSphere = (Hit.ImpactPoint - CapsuleBottomSphereLocation).GetSafeNormal();
			float Deg2 = FMath::RadiansToDegrees(FMath::Acos(-CollisionNormal | ImpactToBottomSphere));

			if (Deg2 <= 90.0f)
				continue;

			if (DamagedCharacter == nullptr)
				continue;
		
			if (DetailCollisionInfo.bHeroSkill)
			{
				if (DamagedCharacter->IsUnitType(EGameDS_UnitType::Enemy) == false)
					continue;
			}
			else
			{
				if (DamagedCharacter->IsUnitType(EGameDS_UnitType::Hero) == false)
					continue;
			}
			
			bHitTarget = true;

			if(AGameDS_HeroCharacter* Hero = Cast<AGameDS_HeroCharacter>(DamagedCharacter))
			{
				if(Hero->GetEvading() == true)
				{
					bHitTarget = false;
					bHitSomething = false;
					continue;
				}
			}
			
			if (DamagedCharacter->GetCrowdControlController())
				DamagedCharacter->GetCrowdControlController()->ApplyCrowdControl(DetailCollisionInfo.CrowdControlIDList, DetailCollisionInfo.OwnerCharacter, StartLocation);

			const float DamageValue = DetailCollisionInfo.DamageValue; 
			UE_LOG(LogTemp, Log, TEXT("%s에게 %f의 데미지를 입혔습니다!"), *DamagedCharacter->GetName(), DamageValue);

			if (DamagedCharacter->OnDamageDelegate.IsBound())
			{
				FGameDS_DamageInfo DamageInfo;
				DamageInfo.DamageType = DetailCollisionInfo.DamageType;
				DamageInfo.DamageValue = DetailCollisionInfo.DamageValue;
				DamageInfo.AttackedUnitID = DetailCollisionInfo.OwnerCharacter->GetUnitID();
				DamageInfo.AttackedSummonUnitID = INDEX_NONE;
				DamageInfo.HitLocation = Hit.ImpactPoint;
				DamageInfo.HitNormal = Hit.ImpactNormal;
				
				if (DamagedCharacter != nullptr)
				{
					UGameDS_StatusEffectController* StatusEffectController = DamagedCharacter->GetStatusEffectController();
					if (StatusEffectController == nullptr)
						continue;

					if (StatusEffectController->IsDamageable() == false)
						continue;

					if (bHitCharacter == false)
						StatusEffectController->AddStatusEffect(102);
				}

				DamagedCharacter->OnDamageDelegate.Broadcast(DamageInfo);
			}
		}
	}

	if (DetailCollisionInfo.bSummon == false)
	{
		UGameInstance::GetSubsystem<UGameDS_GameManager>(GetGameInstance())
			->GetManagerObject<UGameDS_DebugManager>()
			->MakeDebugCollision(DetailCollisionInfo.OwnerCharacter, &CollisionInfo, bHitTarget);

		return bHitTarget;
	}
	else
	{
		UGameInstance::GetSubsystem<UGameDS_GameManager>(GetGameInstance())
			->GetManagerObject<UGameDS_DebugManager>()
			->MakeDebugCollision(DetailCollisionInfo.OwnerCharacter, &CollisionInfo, DetailCollisionInfo.bSummon, NormalDirection, Location, bHitTarget);

		if(bHitTarget == true && bHitCharacter == true)
		{
			return false;
		}

		if (bHitTarget == false && bHitSomething == true)
			return false;

		return true;
	}
}

FCollisionShape UGameDS_BattleSubSystem::MakeCollisionShape(const FGameDS_CollisionInfoDataTable* InCollisionInfoData) const
{
	if (InCollisionInfoData == nullptr)
		return FCollisionShape(); 
	
	switch (InCollisionInfoData->CollisionShape)
	{
	case EGameDS_CollisionShape::Box:
		return FCollisionShape::MakeBox(FVector(InCollisionInfoData->Param1, InCollisionInfoData->Param2, InCollisionInfoData->Param3));
	case EGameDS_CollisionShape::Sphere:
		return FCollisionShape::MakeSphere(InCollisionInfoData->Param2);
	case EGameDS_CollisionShape::Capsule:
	case EGameDS_CollisionShape::Cylinder:
		return FCollisionShape::MakeCapsule(InCollisionInfoData->Param2, InCollisionInfoData->Param1 * 0.5f + InCollisionInfoData->Param2);
	default:
		break;
	}

	return FCollisionShape();
}

void UGameDS_BattleSubSystem::SpawnSummonActor(int32 SummonID, FName SocketName, AGameDS_Character* OwnerCharacter, FVector Scale3D /*= FVector::OneVector*/)
{
	if (OwnerCharacter == nullptr)
		return;

	UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance());
	if (UnitManager == nullptr)
		return;
	
	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DataManager == nullptr)
		return;
	
	const FGameDS_SummonDataTable* SummonData = DataManager->GetDataInRow<FGameDS_SummonDataTable>(SummonID);
	if (SummonData == nullptr)
		return;

	const FGameDS_SkillInfoDataTable* SkillInfoData = DataManager->GetDataInRow<FGameDS_SkillInfoDataTable>(SummonData->SkillInfoID);
	if (SkillInfoData == nullptr)
		return;

	if (SummonData->SummonBP.Get() == nullptr)
		return;

	FVector SpawnLocation = OwnerCharacter->GetActorLocation();

	switch (SummonData->SpawnType)
	{
	case EGameDS_SkillSpawnType::WeaponSocket:
		{
			if (OwnerCharacter->GetMesh()->DoesSocketExist(SocketName))
			{
				SpawnLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
			}
		}
		break;
	case EGameDS_SkillSpawnType::Target:
		{
			if (GetTargetCharacter(OwnerCharacter) != nullptr)
			{
				SpawnLocation = GetTargetCharacter(OwnerCharacter)->GetActorLocation() + FVector(100.f, 100.f, SummonData->SummonInitialDistance * 3.f);
			}
		}
		break;
	case EGameDS_SkillSpawnType::CasterForward:
		{

		}
		break;
	default:
		return;
	}

	FGameDS_SummonUnitSpawnOption SpawnOption;
	SpawnOption.OwnerUnitID = OwnerCharacter->GetUnitID();
	SpawnOption.TargetUnitID = GetTargetCharacter(OwnerCharacter) ? GetTargetCharacter(OwnerCharacter)->GetUnitID() : INDEX_NONE;
	SpawnOption.SummonUnitID = UnitManager->GetNextSummonUnitID();
	SpawnOption.SummonDataID = SummonID;
	SpawnOption.Location = SpawnLocation;
	SpawnOption.Rotator = OwnerCharacter->GetActorRotation();
	UnitManager->SpawnSummonObject(SpawnOption);
}

void UGameDS_BattleSubSystem::SpawnSummonActor(int32 SummonID, FName SocketName, AGameDS_Character* OwnerCharacter, float InDelay)
{

}

void UGameDS_BattleSubSystem::SpawnSummonActorWithAngle(int32 SummonID, FName SocketName, AGameDS_Character* OwnerCharacter, float InAngle)
{
	if (OwnerCharacter == nullptr)
		return;

	UGameDS_UnitManager* UnitManager = UGameInstance::GetSubsystem<UGameDS_UnitManager>(GetGameInstance());
	if (UnitManager == nullptr)
		return;

	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DataManager == nullptr)
		return;

	const FGameDS_SummonDataTable* SummonData = DataManager->GetDataInRow<FGameDS_SummonDataTable>(SummonID);
	if (SummonData == nullptr)
		return;

	const FGameDS_SkillInfoDataTable* SkillInfoData = DataManager->GetDataInRow<FGameDS_SkillInfoDataTable>(SummonData->SkillInfoID);
	if (SkillInfoData == nullptr)
		return;

	if (SummonData->SummonBP.Get() == nullptr)
		return;

	FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
	const FRotator rot(0.f, InAngle, 0.f);

	FVector TargetVector = rot.RotateVector(ForwardVector);

	FVector SpawnLocation = OwnerCharacter->GetActorLocation();

	switch (SummonData->SpawnType)
	{
	case EGameDS_SkillSpawnType::WeaponSocket:
	{
		if (OwnerCharacter->GetMesh()->DoesSocketExist(SocketName))
		{
			SpawnLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
		}
	}
	break;
	case EGameDS_SkillSpawnType::Target:
	{
	}
	break;
	case EGameDS_SkillSpawnType::CasterForward:
	{
		SpawnLocation += OwnerCharacter->GetActorForwardVector() * 500.f;
	}
	break;
	default:
		return;
	}

	uint32 InitialDistance = SummonData->SummonInitialDistance == 0 ? 1 : SummonData->SummonInitialDistance;
	SpawnLocation += TargetVector * InitialDistance;

	FGameDS_SummonUnitSpawnOption SpawnOption;
	SpawnOption.OwnerUnitID = OwnerCharacter->GetUnitID();
	SpawnOption.TargetUnitID = GetTargetCharacter(OwnerCharacter) ? GetTargetCharacter(OwnerCharacter)->GetUnitID() : INDEX_NONE;
	SpawnOption.SummonUnitID = UnitManager->GetNextSummonUnitID();
	SpawnOption.SummonDataID = SummonID;
	SpawnOption.Location = SpawnLocation;
	SpawnOption.Rotator = TargetVector.Rotation();
	UnitManager->SpawnSummonObject(SpawnOption);
}

AGameDS_Character* UGameDS_BattleSubSystem::GetTargetCharacter(AGameDS_Character* Caster)
{
	if (Caster->IsUnitType(EGameDS_UnitType::Enemy))
	{
		AGameDS_Enemy* Enemy = Cast<AGameDS_Enemy>(Caster);
		if (Enemy == nullptr)
			return nullptr;

		return Enemy->GetTarget();
	}
	else
	{
		return Caster;
	}
	
	return nullptr;
}

int UGameDS_BattleSubSystem::CalculrateSpawnAngle(const int32 SummonID, const float InSkillAngle)
{
	float SquaredSkillRadius = 0.f;
	float Degree = 0.f;
	float SquaredMaxDistance = 0.f;

	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DataManager == nullptr)
		return 0;

	const FGameDS_SummonDataTable* SummonData = DataManager->GetDataInRow<FGameDS_SummonDataTable>(SummonID);
	if (SummonData == nullptr)
		return 0;

	const FGameDS_SkillInfoDataTable* SkillInfoData = DataManager->GetDataInRow<FGameDS_SkillInfoDataTable>(SummonData->SkillInfoID);
	if (SkillInfoData == nullptr)
		return 0;

	const FGameDS_CollisionInfoDataTable* CurrentCollisionInfoData = DataManager->GetCollisionInfoData(SkillInfoData->CollisionInfoID);

	if (CurrentCollisionInfoData->CollisionShape == EGameDS_CollisionShape::Sphere)
	{
		SquaredSkillRadius = FMath::Square(CurrentCollisionInfoData->Param2);
	}

	SquaredMaxDistance = FMath::Square(SummonData->SummonMaxDistance);

	float tempAngle = 1.f - ((4.f * SquaredSkillRadius) / (2.f * SquaredMaxDistance));

	Degree = FMath::RadiansToDegrees(FMath::Acos(tempAngle));
	
	int SkillCount = (int)InSkillAngle / Degree;

	return SkillCount;
}

float UGameDS_BattleSubSystem::GetSummonAngleFromDataTable(const int32 SummonID)
{
	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance());
	if (DataManager == nullptr)
		return 0;

	const FGameDS_SummonDataTable* SummonData = DataManager->GetDataInRow<FGameDS_SummonDataTable>(SummonID);
	if (SummonData == nullptr)
		return 0;

	return SummonData->SummonMaxAngle;
}