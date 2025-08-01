// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Controller/GameDS_LockOnController.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Unit/Enemy/GameDS_Enemy.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "ProjectDS/Unit/GameDS_UnitAnimInstance.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacterAnimInstance.h"
#include "ProjectDS/Unit/Enemy/GameDS_EnemyAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ProjectDS/Util/GameDS_Util.h"

UGameDS_LockOnController::UGameDS_LockOnController()
{
	OwnerHeroCharacter = nullptr;
}

void UGameDS_LockOnController::Init(AGameDS_Character* InCharacter, USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	if (InCharacter != nullptr)
	{
		switch (InCharacter->GetUnitType())
		{
		case EGameDS_UnitType::Hero:
		case EGameDS_UnitType::UserHero:
			OwnerCharacter = InCharacter;
			OwnerHeroCharacter = Cast<AGameDS_HeroCharacter>(InCharacter);
			break;
		case EGameDS_UnitType::Enemy:
			OwnerCharacter = Cast<AGameDS_Enemy>(InCharacter);
			break;
		case EGameDS_UnitType::Boss:
			OwnerCharacter = Cast<AGameDS_Boss>(InCharacter);
			break;
		case EGameDS_UnitType::None:
		default:
			break;
		}
		Controller = OwnerCharacter->GetController();
		Camera = InCamera;
		SpringArm = InSpringArm;
		Target = nullptr;
	}
}

bool UGameDS_LockOnController::LockOn(bool bChangeTarget, bool bLeft)
{
	if (bLockOn == false || bChangeTarget == true)
	{
		TArray<FHitResult> HitResults;
		float Length = 1500.0f;

		if(bChangeTarget == false)
			Look();

		if (OwnerCharacter != nullptr)
		{
			FVector StartLocation = OwnerCharacter->GetActorLocation();
			FVector EndLocation = StartLocation + OwnerCharacter->GetActorForwardVector() * Length;
			const UWorld* World = OwnerCharacter->GetWorld();
			FQuat Quat(OwnerCharacter->GetActorRotation());
			FCollisionQueryParams Param(NAME_None, false, OwnerCharacter);
			ECollisionChannel CollisionChannel = COLLISION_CHANNEL_LOCKON;
			bool bHit = World->SweepMultiByChannel(HitResults,
				StartLocation,
				EndLocation,
				Quat,
				CollisionChannel,
				FCollisionShape::MakeBox(FVector(Length, 400.f, 400.f)),
				Param
			);

			if (bHit)
			{
				TArray<AGameDS_Character*> Characters;

				for (FHitResult HitResult : HitResults)
				{
					if (AGameDS_Character* LockOnCharacter = Cast<AGameDS_Character>(HitResult.GetActor()))
					{
						if (LockOnCharacter->IsDied() == true)
							continue;
						if (bChangeTarget == false)
						{
							SetTarget(LockOnCharacter);
							bLockOn = true;
							OwnerCharacter->bUseControllerRotationYaw = true;
							if (Target != nullptr)
							{
								Target->ToggleLockOnImage();
								DeadDelegateHandle = Target->OnDeadDelegate.AddUObject(this, &UGameDS_LockOnController::LockOff);
								DespawnDelegateHandle = Target->OnDespawnDelegate.AddUObject(this, &UGameDS_LockOnController::LockOff);
							}
							return true;
						}
						else
						{
							if (Target == nullptr)
								return false;
							
							if (Target == LockOnCharacter)
								continue;

							FVector ToTargetVector =  Target->GetActorLocation() - OwnerCharacter->GetActorLocation();
							FVector ToCandidateVector = LockOnCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation();
							FVector CrossResult = FVector::CrossProduct(ToTargetVector, ToCandidateVector);
							TArray<AGameDS_Character> CArray;
							
							if (bLeft == true)
							{
								bool bLocateLeft = CrossResult.Z < 0;
								if(bLocateLeft == true)
									Characters.Add(LockOnCharacter);
							}
							else
							{
								bool bLocateRight = CrossResult.Z > 0;
								if (bLocateRight == true)
									Characters.Add(LockOnCharacter);
							}
						}
					}
				}

				if (Characters.IsEmpty() == true)
					return false;

				AGameDS_Character* NextTarget = nullptr;
				float MinDist2Target = -1.f;
				for (AGameDS_Character* Character : Characters)
				{
					float CurrentDist2Character = FVector::Dist2D(Character->GetActorLocation(), OwnerCharacter->GetActorLocation());
					if (MinDist2Target < 0.f || CurrentDist2Character < MinDist2Target)
					{
						MinDist2Target = CurrentDist2Character;
						NextTarget = Character;
					}
				}

				if (NextTarget != nullptr)
				{
					//기존 타겟 락온 해제
					Target->ToggleLockOnImage();
					Target->OnDeadDelegate.Remove(DeadDelegateHandle);
					Target->OnDespawnDelegate.Remove(DespawnDelegateHandle);

					SetTarget(NextTarget);
					Target->ToggleLockOnImage();
					DeadDelegateHandle = Target->OnDeadDelegate.AddUObject(this, &UGameDS_LockOnController::LockOff);
					DespawnDelegateHandle = Target->OnDespawnDelegate.AddUObject(this, &UGameDS_LockOnController::LockOff);
					return true;
				}
			}
		}
	}
	return false;
}

void UGameDS_LockOnController::LockOff()
{
	if (Target != nullptr)
	{		
		if(Target->IsDied() == true)
		{
			if(LockOn(true, true) == true)
				return;
			if(LockOn(true, false) == true)
				return;
		}
		
		Target->ToggleLockOnImage();
		Target->OnDeadDelegate.Remove(DeadDelegateHandle);
		Target->OnDespawnDelegate.Remove(DespawnDelegateHandle);
		bLockOn = false;
		Target = nullptr;
		if (OwnerCharacter != nullptr)
		{
			OwnerCharacter->bUseControllerRotationYaw = false;
		}
	}
}

void UGameDS_LockOnController::Update(float DeltaTime)
{
	if (bLockOn == true)
	{
		if (OwnerHeroCharacter != nullptr)
		{
			if (Target == nullptr)
			{
				LockOff();
				return;
			}

			FVector LockOnCharacterLocation = Target->GetMesh()->GetSocketLocation(Target->GetLockOnSocketName());
			FRotator LockOnRotation = (LockOnCharacterLocation - OwnerCharacter->GetActorLocation()).Rotation();
			float Length = (LockOnCharacterLocation - OwnerCharacter->GetActorLocation()).Size();
			Length = FMath::Clamp(Length, 100.0f, 500.0f);

			const float PitchPercentage = (Length - 100.0f) * 0.0025f;
			float PlusMaxPitch = PitchPercentage * 40.f;
			PlusMaxPitch += 30.0f;

			LockOnRotation.Pitch = FMath::Clamp(LockOnRotation.Pitch, -CameraLockOnMaxPitchValue, PlusMaxPitch);

			FRotator ControlRotation = Controller->GetControlRotation();
			ControlRotation = FMath::Lerp(ControlRotation, LockOnRotation, DeltaTime * 10.0f);
			
			Controller->SetControlRotation(ControlRotation);

			if (OwnerHeroCharacter->GetRunning() == false)
			{
				FRotator YawRotator = FRotator(0.f, ControlRotation.Yaw, 0.f);
				OwnerCharacter->SetActorRotation(YawRotator);
			}
			else
			{
				FVector PlayerMovementDirection = OwnerCharacter->GetVelocity().GetSafeNormal2D();
				FRotator PlayerMovementRotation = PlayerMovementDirection.Rotation();
				OwnerCharacter->SetActorRotation(PlayerMovementRotation);
			}
		}
	}
}

void UGameDS_LockOnController::Look()
{
	bLook = true;
	FRotator Rotation = OwnerCharacter->GetActorRotation();
	FRotator ControlRotation = Controller->GetControlRotation();
	ControlRotation.Yaw = Rotation.Yaw;
	
	Controller->SetControlRotation(Rotation);
}

void UGameDS_LockOnController::SetTarget(AGameDS_Character* InTarget)
{
	Target = Cast<AGameDS_Enemy>(InTarget);
}
