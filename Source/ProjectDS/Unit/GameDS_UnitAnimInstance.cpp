#include "GameDS_UnitAnimInstance.h"

#include "GameDS_Character.h"
#include "Controller/GameDS_StatusEffectController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ProjectDS/Manager/GameDS_BattleSubSystem.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"

UGameDS_UnitAnimInstance::UGameDS_UnitAnimInstance()
{
}

void UGameDS_UnitAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	InitAnimationAlias();
}

void UGameDS_UnitAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerPawn == nullptr)
	{
		OwnerPawn = TryGetPawnOwner();
		OwnerCharacter = Cast<AGameDS_Character>(TryGetPawnOwner());
	}

	if (IsValid(OwnerPawn))
	{
		SpeedValue = OwnerPawn->GetVelocity().Size();

		const FVector SpeedNormalVector = OwnerPawn->GetVelocity().GetSafeNormal2D();
		Velocity.X = SpeedNormalVector.X;
		Velocity.Y = SpeedNormalVector.Y;

		const FVector2D Forward2D = {OwnerPawn->GetActorForwardVector().X, OwnerPawn->GetActorForwardVector().Y};
		bMoveLeft = FVector2D::CrossProduct(Forward2D, Velocity) < 0.0f;
	}
}

void UGameDS_UnitAnimInstance::InitAnimationAlias()
{
	if (OwnerCharacter == nullptr)
		return;

	if (AnimationAlias.IsEmpty())
		return;	

	UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(OwnerPawn->GetGameInstance());
	if (DataManager == nullptr)
		return;

	const UDataTable* SkillDataTable = DataManager->GetDataTable<FGameDS_SkillDataTable>();
	if (SkillDataTable == nullptr)
		return;

	for (const auto& AliasData : AnimationAlias)
	{
		const int32 SkillID = AliasData.SkillID;

		const FGameDS_SkillDataTable* SkillData = SkillDataTable->FindRow<FGameDS_SkillDataTable>(FName(FString::FromInt(SkillID)), TEXT(""));
		if (SkillData == nullptr)
			continue;

		AliasCachingMap.Add(SkillID, *SkillData);
		SkillIDList.Add(SkillID);
	}
}

const FGameDS_SkillDataTable* UGameDS_UnitAnimInstance::GetSkillData(EGameDS_UnitAnimType InAnimType, int32 Index/* = 0*/)
{
	int32 SkillID = GetSkillID(InAnimType, Index);
	return AliasCachingMap.Find(SkillID);
}

int32 UGameDS_UnitAnimInstance::GetSkillID(EGameDS_UnitAnimType InAnimType, int32 Index/*= 0*/)
{
	TArray<TArray<FGameDS_AnimationAliasData>::ElementType> AnimList = AnimationAlias.FilterByPredicate(
		[InAnimType](const FGameDS_AnimationAliasData& AliasData)
		{
			return AliasData.AnimType == InAnimType;
		});

	if (Index < AnimList.Num())
		return AnimList[Index].SkillID;

	return INDEX_NONE;
}

void UGameDS_UnitAnimInstance::GetSkillID(EGameDS_UnitAnimType InAnimType, TArray<int32>& SkillIDListRef)
{
	for (auto AliasData : AnimationAlias)
	{
		if (InAnimType == AliasData.AnimType)
			SkillIDListRef.Add(AliasData.SkillID);
	}
}

void UGameDS_UnitAnimInstance::PlayMontage(EGameDS_UnitAnimType InAnimType, int32 Index/* = 0*/)
{
	int32 SkillID = GetSkillID(InAnimType, Index);
	PlayMontage(SkillID);
}

void UGameDS_UnitAnimInstance::PlayMontage(int32 SkillID)
{
	const FGameDS_SkillDataTable* FindSkillData = AliasCachingMap.Find(SkillID);
	if (FindSkillData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s : PlayMontage를 하려 했으나 해당 AnimType의 정보가 없습니다"), *GetOwningActor()->GetName());
		return;
	}
	if (FindSkillData->SkillMontage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s : PlayMontage를 하려 했으나 Montage가 없습니다"), *GetOwningActor()->GetName());
		return;
	}

	Montage_Play(FindSkillData->SkillMontage);
}

void UGameDS_UnitAnimInstance::PlayCCMotion(EGameDS_CrowdControlType InCCType)
{
	switch(InCCType)
	{
		case EGameDS_CrowdControlType::None: break;
		case EGameDS_CrowdControlType::PushBack: break;
		case EGameDS_CrowdControlType::NormalKnockBack:
			{
				if (NormalKnockBackMotion)
					Montage_Play(NormalKnockBackMotion);
			}
		break;
		case EGameDS_CrowdControlType::HeavyKnockBack:
			{
				if (HeavyKnockBackMotion)
					Montage_Play(HeavyKnockBackMotion);
			}
		break;
		case EGameDS_CrowdControlType::Stun: break;
		case EGameDS_CrowdControlType::Num: break;
		default: ;
	}

}

void UGameDS_UnitAnimInstance::ChangeBattleMode(bool bActive)
{
	bBattleMode = bActive;
}

void UGameDS_UnitAnimInstance::EndAttackNotify()
{
	EndAttackDelegate.ExecuteIfBound();
}

void UGameDS_UnitAnimInstance::EndHitNotify()
{
	EndHitDelegate.ExecuteIfBound();
}

void UGameDS_UnitAnimInstance::EndEvadeNotify()
{
	EndEvadeDelegate.ExecuteIfBound();
}

void UGameDS_UnitAnimInstance::EndModeChangeNotify()
{
	EndModeChangeDelegate.ExecuteIfBound();
}

bool UGameDS_UnitAnimInstance::SpawnSkillCollision(int32 InSkillInfoID, bool bNeutral)
{
	if (OwnerCharacter == nullptr)
		return false;

	UGameDS_BattleSubSystem* BattleSubSystem = UGameInstance::GetSubsystem<UGameDS_BattleSubSystem>(OwnerCharacter->GetGameInstance());
	if (BattleSubSystem == nullptr)
		return false;

	const FGameDS_DetailCollisionInfo& DetailCollisionInfo = BattleSubSystem->MakeSkillCollision(InSkillInfoID, OwnerCharacter, false, bNeutral);
	if (DetailCollisionInfo.bValidData == false)
		return false;

	return BattleSubSystem->OnAttackCheck(DetailCollisionInfo, OwnerCharacter->GetActorForwardVector());
}

bool UGameDS_UnitAnimInstance::SpawnSkillCollisionWithCharging(int32 InSkillInfoID, bool bNeutral, float InChargingTime)
{
	if (OwnerCharacter == nullptr)
		return false;

	UGameDS_BattleSubSystem* BattleSubSystem = UGameInstance::GetSubsystem<UGameDS_BattleSubSystem>(OwnerCharacter->GetGameInstance());
	if (BattleSubSystem == nullptr)
		return false;

	FGameDS_DetailCollisionInfo DetailCollisionInfo = BattleSubSystem->MakeSkillCollision(InSkillInfoID, OwnerCharacter, false, bNeutral);

	float TempDamageIncreaseValue = 10.0f;

	DetailCollisionInfo.DamageValue += InChargingTime * 2.0f;

	if (DetailCollisionInfo.bValidData == false)
		return false;

	return BattleSubSystem->OnAttackCheck(DetailCollisionInfo);
}

void UGameDS_UnitAnimInstance::SpawnSummonActor(int32 InSummonID, FName SocketName/* = TEXT("")*/)
{
	if (OwnerCharacter == nullptr)
		return;

	UGameDS_BattleSubSystem* BattleSubSystem = UGameInstance::GetSubsystem<UGameDS_BattleSubSystem>(OwnerCharacter->GetGameInstance());
	if (BattleSubSystem == nullptr)
		return;

	BattleSubSystem->SpawnSummonActor(InSummonID, SocketName, OwnerCharacter, FVector(SummonScaleValue));

	SummonScaleValue = 1.f;
}

void UGameDS_UnitAnimInstance::SpawnSummonActorWithAngle(int32 InSummonID, float InAngle, FName SocketName)
{
	if (OwnerCharacter == nullptr)
		return;

	UGameDS_BattleSubSystem* BattleSubSystem = UGameInstance::GetSubsystem<UGameDS_BattleSubSystem>(OwnerCharacter->GetGameInstance());
	if (BattleSubSystem == nullptr)
		return;

	BattleSubSystem->SpawnSummonActorWithAngle(InSummonID, SocketName, OwnerCharacter, InAngle);

	SummonScaleValue = 1.f;
}

void UGameDS_UnitAnimInstance::EndDieAnimCallBack()
{
	if (OwnerCharacter == nullptr)
		return;

	OwnerCharacter->GetWorld()->DestroyActor(OwnerCharacter);
}

void UGameDS_UnitAnimInstance::AddStatusEffectNotify(int32 InDataID, float InDuration)
{
	if (OwnerCharacter == nullptr)
		return;

	UGameDS_StatusEffectController* StatusEffectController = OwnerCharacter->GetStatusEffectController();
	if (StatusEffectController == nullptr)
		return;

	StatusEffectController->AddStatusEffect(InDataID);
}

EPhysicalSurface UGameDS_UnitAnimInstance::GetGroundPhysMaterial(EGameDS_FootStepType InFootStepType, FVector& OutLocation)
{
	if (OwnerCharacter == nullptr)
		return SurfaceType62;

	FName SocketName;
	switch (InFootStepType)
	{
	case EGameDS_FootStepType::None:
		SocketName = TEXT("None");
		break;
	case EGameDS_FootStepType::Left:
		SocketName = OwnerCharacter->GetLeftFootSocketName();
		break;
	case EGameDS_FootStepType::Right:
		SocketName = OwnerCharacter->GetRightFootSocketName();
		break;
	case EGameDS_FootStepType::Both:
		SocketName = OwnerCharacter->GetRootSocketName();
		break;
	default: ;
	}

	const USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
	if (CharacterMesh == nullptr)
		return SurfaceType62;

	const FVector SocketLocation = CharacterMesh->GetSocketLocation(SocketName);
	const FVector TopLocation = SocketLocation + FVector::UpVector * 150.0f;
	const FVector BottomLocation = SocketLocation + FVector::DownVector * 100.0f;

	FHitResult Hit;
	FCollisionQueryParams TraceParams;
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.bTraceComplex = false;
	TraceParams.AddIgnoredActor(OwnerCharacter);
	bool bHitComplete = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		Hit,
		TopLocation,
		BottomLocation,
		ECC_Visibility,
		TraceParams);

	if (bHitComplete == false)
		return SurfaceType62;

	OutLocation = Hit.Location;

	if (Hit.PhysMaterial.Get() == nullptr)
		return SurfaceType1;

	return Hit.PhysMaterial.Get()->SurfaceType;
}

void UGameDS_UnitAnimInstance::CachingAliasData(int32 SkillID, FGameDS_SkillDataTable* SkillData)
{
	if (SkillID == INDEX_NONE)
		return;

	if (SkillData == nullptr)
		return;

	AliasCachingMap.Add(SkillID, *SkillData);
	SkillIDList.Add(SkillID);
}

UAnimMontage* UGameDS_UnitAnimInstance::GetMontageBySkillID(int32 SkillID)
{
	const FGameDS_SkillDataTable* FindSkillData = AliasCachingMap.Find(SkillID);
	if (FindSkillData == nullptr)
		return nullptr;

	return FindSkillData->SkillMontage;
}

bool UGameDS_UnitAnimInstance::IsLeftMove() const
{
	return true;
}

void UGameDS_UnitAnimInstance::ResetAliasData()
{
	AnimationAlias.Reset();
	AliasCachingMap.Reset();
	SkillIDList.Reset();
}

bool UGameDS_UnitAnimInstance::IsValidSkillData(EGameDS_UnitAnimType InAnimType, int32 Index/* = 0*/)
{
	return GetSkillID(InAnimType, Index) != INDEX_NONE;
}
