#include "GameDS_Summon.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/Manager/GameDS_SummonManager.h"
#include "ProjectDS/GameInstanceHolder.h"

AGameDS_Summon::AGameDS_Summon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGameDS_Summon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bAfterDelay == false)
	{
		CheckDelay(DeltaSeconds);
		return;
	}

	UpdateDuration(DeltaSeconds);

	CheckDistance();

	if (bHit == true)
		return;

	SetActorLocation(GetActorLocation() + GetActorForwardVector() * SummonSpeed * DeltaSeconds);
	
	if (SummonType == EGameDS_SummonType::Default)
	{
	}

	if (bActive == false)
		return;
	
	if (UGameDS_SummonManager* SummonManager = FGameInstanceHolder::GetManager<UGameDS_SummonManager>())
	{
		AGameDS_Summon* NearestSummon = SummonManager->MultiSpawnSummonNearbyCharacter(Target, SkillID);
		bSoundOn = false;

		if (NearestSummon != nullptr)
		{
			if (NearestSummon->GetName().Equals(this->GetName()))
				bSoundOn = true;
		}
	}

	if (SummonCollisionCheckDelegate.IsBound())
	{
		bActive = SummonCollisionCheckDelegate.Execute(DetailCollisionInfo, GetActorForwardVector(), GetActorLocation(), bCollisionDisappearance);
	}

	if (bActive == false && bOnlyDespawnByDuration == false)
	{
		OnDespawn();
	}
}

void AGameDS_Summon::Init(AGameDS_Character* InOwner, AGameDS_Character* InTarget, const FGameDS_DetailCollisionInfo& InDetailCollisionInfo,
	const FGameDS_SummonDataTable* InSummonData)
{
	if (InOwner == nullptr)
		return;
	
	if (InSummonData == nullptr)
		return;

	UGameDS_DataManager* DataManager = GetGameInstance()->GetSubsystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	UGameDS_UnitManager* UnitManager = GetGameInstance()->GetSubsystem<UGameDS_UnitManager>();
	if (UnitManager == nullptr)
		return;

	const FGameDS_SkillInfoDataTable* SkillInfoData = DataManager->GetDataInRow<FGameDS_SkillInfoDataTable>(InSummonData->SkillInfoID);
	if (SkillInfoData == nullptr)
		return;

	SummonOwnerUnitID = InOwner->GetUnitID();
	SummonUnitID = UnitManager->GetNextSummonUnitID(); 

	SummonOwner = InOwner;
	Target = InTarget;

	DetailCollisionInfo = InDetailCollisionInfo;
	
	SummonType = InSummonData->SummonType;
	Damage = SkillInfoData->DamageValue;
	SummonSpeed = InSummonData->SummonSpeed;
	SummonDuration = InSummonData->SummonDuration;
	SummonMaxDistance = InSummonData->SummonMaxDistance;
	SummonDelay = InSummonData->SummonDelay;
	InitialLocation = GetActorLocation();
	bCollisionDisappearance = InSummonData->bCollisionDisappearance;
	bOnlyDespawnByDuration = InSummonData->bOnlyDespawnByDuration;
	bActive = true;
	SkillID = InSummonData->SkillInfoID;
	bSoundOn = false;
}

void AGameDS_Summon::UpdateDuration(float DeltaTime)
{
	if (SummonElapsedTime < SummonDuration)
	{
		SummonElapsedTime += DeltaTime;
	}
	else
	{
		OnDespawn();
		SummonElapsedTime = 0.f;
	}
}

void AGameDS_Summon::CheckDistance()
{
	if (bOnlyDespawnByDuration == true)
		return;

	if (SummonType != EGameDS_SummonType::Default)
		return;

	if (SummonMaxDistance <= 0.0f)
		return;

	float Distance = FVector::Dist2D(InitialLocation, GetActorLocation());

	if (Distance >= SummonMaxDistance)
	{
		OnDespawn();
	}
}

void AGameDS_Summon::CheckDelay(float DeltaTime)
{
	if (SummonElapsedTime < SummonDelay)
	{
		SummonElapsedTime += DeltaTime;
		if (Target == nullptr)
			return;

		FVector TargetVector = Target->GetActorLocation() - GetActorLocation();
		SetActorRotation(TargetVector.Rotation());
	}
	else
	{
		SummonElapsedTime = 0.0f;
		bAfterDelay = true;
	}
}

void AGameDS_Summon::OnDespawn_Implementation()
{
	SummonDespawnDelegate.ExecuteIfBound(this);
}

void AGameDS_Summon::OnSystemFinished_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Test!!"));
}