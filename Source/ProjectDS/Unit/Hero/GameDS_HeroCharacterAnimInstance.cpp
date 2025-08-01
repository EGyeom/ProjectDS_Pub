// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Hero/GameDS_HeroCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "ProjectDS/Manager/GameDS_UnitManager.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Unit/Controller/GameDS_CrowdControlController.h"
#include "ProjectDS/Util/GameDS_Util.h"

UGameDS_HeroCharacterAnimInstance::UGameDS_HeroCharacterAnimInstance()
{
	AnimNotifyState = EGameDS_HeroAnimNotifyState::Moveable;
	bAttackable = true;
	bMoveable = true;
	bDrinking = false;
	BlendWeight = 1.0f;
	Jump_MaxZValue = -9999.f;
	JumpHeight = 0.0f;
	bDefenseAttack = false;
}

void UGameDS_HeroCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UGameDS_HeroCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerPawn != nullptr)
	{
		OwnerHeroCharacter = Cast<AGameDS_HeroCharacter>(OwnerPawn);
		Speed = OwnerPawn->GetVelocity().Size();
		if (OwnerHeroCharacter != nullptr)
		{
			bJumping = OwnerHeroCharacter->GetIsJumpingStart();
			bFalling = !OwnerCharacter->GetMovementComponent()->IsMovingOnGround();
			BlendWeight = (bJumping || bFalling) ? 0.2f : 1.0f;
			bDefensing = OwnerHeroCharacter->bDefensing;
			Horizontal = OwnerHeroCharacter->LeftRightValue;
			Vertical = OwnerHeroCharacter->UpDownValue;
			bRunning = OwnerHeroCharacter->GetRunning();
			bLockOn = OwnerHeroCharacter->bUseControllerRotationYaw;
			bPressedInput = OwnerHeroCharacter->bPressedInput;

			if (OwnerHeroCharacter->IsInteraction())
			{
				if (Montage_IsPlaying(nullptr) == true)
				{
					StopAllMontages(0.3f);
					bMoveable = true;
				}
			}

			if (bRunning == true)
			{
				bLockOn = false;
			}
			
			if (bFalling == true)
			{
				if (Jump_MaxZValue < -9000.f && OwnerPawn->GetVelocity().Z < 0)
				{
					Jump_MaxZValue = OwnerPawn->GetActorTransform().GetLocation().Z;
					bJumpEndAnim = false;
				}
			}
			else
			{
				if (Jump_MaxZValue > -9000.f)
				{
					JumpHeight = Jump_MaxZValue - OwnerPawn->GetActorTransform().GetLocation().Z;
					Jump_MaxZValue = -9999.f;
					if (JumpHeight >= 400.f)
					{
						bJumpEndAnim = true;
					}
				}
			}
		}
	}
}

void UGameDS_HeroCharacterAnimInstance::InitAnimationAlias()
{
	Super::InitAnimationAlias();
}

void UGameDS_HeroCharacterAnimInstance::EndDieAnimCallBack()
{
	UGameDS_GameManager* GameManager = FGameInstanceHolder::GetSubSystem<UGameDS_GameManager>();
	if (GameManager == nullptr)
		return;

	GameManager->RequestRespawnAllUnit();
	
	Super::EndDieAnimCallBack();
}

void UGameDS_HeroCharacterAnimInstance::SetAnimationAlias(const FGameDS_HeroSkillSetDataTable* InSkillSet)
{
	AnimationAlias.Empty();
	
	for (int32 SkillID : InSkillSet->SkillSet)
	{
		FGameDS_AnimationAliasData AnimAliasData;

		AnimAliasData.SkillID = SkillID;
		switch (SkillID % 10)
		{
		case 1:
			AnimAliasData.AnimType = EGameDS_UnitAnimType::NormalAttack;
			break;
		case 2:
			AnimAliasData.AnimType = EGameDS_UnitAnimType::SpecialAttack;
			break;
		default:
			AnimAliasData.AnimType = EGameDS_UnitAnimType::Die;
		}
		AnimationAlias.Add(AnimAliasData);
	}

	SetWeaponType(InSkillSet->WeaponType);
	InitAnimationAlias();
}

void UGameDS_HeroCharacterAnimInstance::PlayAttackMontage()
{
	bAttackable = false;
	bMoveable = false;
	AnimNotifyState = EGameDS_HeroAnimNotifyState::Attacking;
	PlayMontage(EGameDS_UnitAnimType::NormalAttack);
}
void UGameDS_HeroCharacterAnimInstance::PlayStrongAttackMontage()
{
	bAttackable = false;
	bMoveable = false;
	AnimNotifyState = EGameDS_HeroAnimNotifyState::Attacking;
	PlayMontage(EGameDS_UnitAnimType::SpecialAttack);
}

void UGameDS_HeroCharacterAnimInstance::ForceEvadeSetting()
{
	bMoveable = true;
	bAttackable	= true;
}

void UGameDS_HeroCharacterAnimInstance::OnEvade()
{
	if (OwnerHeroCharacter == nullptr)
		return;

 	if (bMoveable == false)
		return;
	
	if (bAttackable == false)
		return;
	
	if (bEvading == true)
		return;

	if (OwnerHeroCharacter->GetMoving())
	{
		bMoving = true;
	}
	else
	{
		bMoving = false;
	}

	bMoveable = false; 
	bAttackable = false;
	bEvading = true;
}

void UGameDS_HeroCharacterAnimInstance::EndEvade()
{
	bMoveable = true;
	bAttackable = true;
	bEvading = false;
	OwnerHeroCharacter->OnEndEvade();
	StopAllMontages(0.5f);
}

void UGameDS_HeroCharacterAnimInstance::BPCallableStopAllMontages(float Seconds /* = 0.f; */)
{
	StopAllMontages(Seconds);
}

void UGameDS_HeroCharacterAnimInstance::EndDrinkNotify()
{
	bDrinking = false;
}

void UGameDS_HeroCharacterAnimInstance::DrinkNotify()
{
	if (OwnerHeroCharacter == nullptr)
		return;

	OwnerHeroCharacter->DrinkPotion();
}

FName UGameDS_HeroCharacterAnimInstance::GetAttackMontageName(int32 SectionIndex)
{
	return FName(*FString::Printf(TEXT("Attack%d"), SectionIndex));
}

void UGameDS_HeroCharacterAnimInstance::JumpToSection(FString AttackType, int32 SectionIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("%d"), SectionIndex);
	FName Name = GetAttackMontageName(SectionIndex);
	UAnimMontage* AnimMontage = AttackType.Equals("NormalAttack") ? AttackMontage : StrongAttackMontage;
	Montage_JumpToSection(Name, AnimMontage);
}

bool UGameDS_HeroCharacterAnimInstance::GetAttackable()
{
	return bAttackable;
}

bool UGameDS_HeroCharacterAnimInstance::GetMoveable()
{
	return bMoveable;
}

bool UGameDS_HeroCharacterAnimInstance::InitMontages(const TMap<FString, UAnimMontage*>& InMontages)
{
	if (*InMontages.Find(TEXT("NormalAttack")) == AttackMontage)
		return false;

	if (InMontages.IsEmpty() == false)
	{
		OwnerCharacter->OnDamageDelegate.AddLambda([this](const FGameDS_DamageInfo& InDamageInfo)
			{
				if (bEvading == true)
					return;

				if (OwnerCharacter->IsDied())
					return;

				if (bDefensing == true)
				{
					SetDefenseAttack(true);
				}

				if (OwnerCharacter->GetCrowdControlController()->IsPlayingCCMotion() == false)
				{
					StopAllMontages(0.1f);
					SetHit(true);
				}

				bAttackable = false;
				bMoveable = false;
				bDrinking = false;
			}
		);
		
		EndHitDelegate.BindLambda([this]
			{
				if (bDefenseAttack == true)
				{
					SetDefenseAttack(false);
				}
				SetHit(false);
				bAttackable = true;
				bMoveable = true;
				OwnerHeroCharacter->SetAttacking(false);
			}
		);

		AttackMontage = *InMontages.Find(TEXT("NormalAttack"));
		StrongAttackMontage = *InMontages.Find(TEXT("StrongAttack"));
	}

	if (AttackMontage == nullptr)
		return false;

	return true;
}

void UGameDS_HeroCharacterAnimInstance::SetDefenseAttack(bool InDefense)
{
	bDefenseAttack = InDefense;
	if (bDefenseAttack == true)
	{
		OwnerHeroCharacter->DefenseAttack();
	}
	else
	{
		OwnerHeroCharacter->EndDefenseAttack();
	}
}

void UGameDS_HeroCharacterAnimInstance::AnimNotify_JumpStart()
{
	OwnerHeroCharacter->Jump();
}
