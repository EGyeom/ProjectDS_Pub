// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../GameDS_UnitDefine.h"
#include "ProjectDS/Unit/GameDS_UnitAnimInstance.h"
#include "GameDS_HeroCharacterAnimInstance.generated.h"

class AGameDS_HeroCharacter;

UCLASS()
class PROJECTDS_API UGameDS_HeroCharacterAnimInstance : public UGameDS_UnitAnimInstance
{
	GENERATED_BODY()
public:
	UGameDS_HeroCharacterAnimInstance();
	
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void InitAnimationAlias() override;

	virtual void EndDieAnimCallBack() override;

	void SetWeaponType(EGameDS_WeaponType InWeaponType) { WeaponType = InWeaponType; }
	void SetAnimationAlias(const FGameDS_HeroSkillSetDataTable* InSkillSet);
	void PlayAttackMontage();
	void PlayStrongAttackMontage();
	void ForceEvadeSetting();
	void OnEvade();
	void EndEvade();
	FName GetAttackMontageName(int32 SectionIndex);
	void JumpToSection(FString AttackType, int32 SectionIndex);
	bool GetAttackable();
	bool GetMoveable();
	bool GetEvading() { return bEvading; }
	bool IsDrink() const { return bDrinking; }
	void SetDrink(bool bInDrinkState) { bDrinking = bInDrinkState; }
	bool InitMontages(const TMap<FString, UAnimMontage*>& InMontages);
	void SetDefenseAttack(bool InDefense);

	UFUNCTION(BlueprintCallable)
	void BPCallableStopAllMontages(float Seconds = 0.f);

	UFUNCTION(BlueprintCallable)
	void EndDrinkNotify();
	UFUNCTION(BlueprintCallable)
	void DrinkNotify();
private:

	UFUNCTION()
	void AnimNotify_JumpStart();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bJumping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bDefensing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bDefenseAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bAttackable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bMoveable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bPressedInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	float Horizontal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	float Vertical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bLockOn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bEvading;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bMoving;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bRunning;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	float BlendWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bDrinking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	float Jump_MaxZValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	float JumpHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	UAnimMontage* FirstSkillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	UAnimMontage* SecondSkillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	UAnimMontage* EvadeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = true))
	UAnimMontage* StrongAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	EGameDS_HeroAnimNotifyState AnimNotifyState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	EGameDS_WeaponType WeaponType;

	UPROPERTY()
	AGameDS_HeroCharacter* OwnerHeroCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (AllowPrivateAccess = true))
	bool bJumpEndAnim;
};
