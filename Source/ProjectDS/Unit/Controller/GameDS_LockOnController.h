// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameDS_LockOnController.generated.h"

class AGameDS_Character;
class AGameDS_HeroCharacter;
class UWidgetComponent;
class UUserWidget;
class UGameDS_UnitAnimInstance;
class AController;
class UCameraComponent;
class USpringArmComponent;
class AGameDS_Enemy;

UCLASS()
class PROJECTDS_API UGameDS_LockOnController : public UObject
{
	GENERATED_BODY()
	UGameDS_LockOnController();

public:
	void Init(AGameDS_Character* InCharacter, USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	bool IsLockOn() const { return bLockOn; }
	bool LockOn(bool bChangeTarget, bool bLeft);
	void LockOff();
	void Update(float DeltaTime);
	void Look();
	void SetTarget(AGameDS_Character* InTarget);

	AGameDS_Character* GetOwnerCharacter() const { return OwnerCharacter; }
	AGameDS_Enemy* GetTarget() const { return Target; }
	UGameDS_UnitAnimInstance* GetOwnerAnimInstance() const { return OwnerAnimInstance; }
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UGameDS_UnitAnimInstance* OwnerAnimInstance;

	UPROPERTY()
	AGameDS_HeroCharacter* OwnerHeroCharacter;
	
	UPROPERTY()
	AGameDS_Character* OwnerCharacter;

	UPROPERTY()
	AGameDS_Enemy* Target;
	
	UPROPERTY()
	UCameraComponent* Camera;


	UPROPERTY()
	AController* Controller;

	UPROPERTY()
	USpringArmComponent* SpringArm;

	FDelegateHandle DeadDelegateHandle;
	FDelegateHandle DespawnDelegateHandle;
	
	bool bLockOn = false;
	bool bLook = false;

	float CameraInitialPitchValue = 0.f;
	float CameraLockOnMaxPitchValue = 30.f;
};
