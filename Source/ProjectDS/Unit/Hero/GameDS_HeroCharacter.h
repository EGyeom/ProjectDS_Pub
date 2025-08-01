// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "../GameDS_Character.h"
#include "GameDS_HeroCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UGameDS_HeroCharacterAnimInstance;
class UGameDS_UnitAnimInstance;
struct FGameDS_WeaponDataTable;

class UGameDS_LockOnController;
class UGameDS_InputController;
class UGameDS_InventoryController;
struct FGameDS_HeroSkillSetDataTable;
struct FEnhancedActionKeyMapping;
enum class EGameDS_GenderType : uint8;

DECLARE_MULTICAST_DELEGATE_OneParam(FCharacterInventoryUpdateDelegate, UGameDS_InventoryController*)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterSpawnDelegate);

UENUM(BlueprintType)
enum class EGameDS_HeroNextActionState : uint8
{
	None = 0,
	Idle,
	Attack,
	Run,
	Evade,
	StrongAttack
};

enum class EGameDS_HeroActionState : uint8;

UCLASS()
class PROJECTDS_API AGameDS_HeroCharacter : public AGameDS_Character
{
	GENERATED_BODY()
public:
	AGameDS_HeroCharacter();
	
	FCharacterInventoryUpdateDelegate InventoryUpdateDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "SpawnSignals")
	FCharacterSpawnDelegate SpawnDelegate;
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo, const FGameDS_HeroStatInfo& InHeroStatInfo) override;

	virtual void OnDamaged(const FGameDS_DamageInfo& InDamageInfo) override;

	virtual bool IsUnitType(EGameDS_UnitType InUnitType) override;

	void LoadData();
	void SaveINI();

	void UpdateInventory();
	
	UFUNCTION(BlueprintCallable)
	void ToggleTargetLock();
	UFUNCTION(BlueprintCallable)
	void OnInteraction(bool bEnable, int32 InInteractionID = -1);
	UFUNCTION(BlueprintCallable)
	bool IsInteractionArea() const { return bInteractionArea; }
	UFUNCTION(BlueprintCallable)
	bool IsInteraction() const { return bInteraction; }
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetInteractionCamera(bool bEnable);
	UFUNCTION(BlueprintCallable)
	void InitInteractionSetting(bool bActive);

	UFUNCTION(BlueprintCallable)
	int32 GetLastInteractionID() const { return LastInteractionID; }

	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void MoveEnd();

	void OnAttack();
	void OnStrongAttack();
	void OnStrongAttackReleased();
	void OnDefense();
	void OnDefenseEnd();
	void OnPressedSpace();
	void OnReleasedSpace();
	void OnEndEvade();
	void OnUpdateUI();
	void OnEndExhaustion();

	void OnChangeItem(const EGameDS_ArrowKey Arrow);

	void PreJump();
	void Jump() override;
	void StopJumping() override;
	bool GetIsJumpingStart() const { return bJumping; }

	FName GetWeaponSocketName() const;

	UFUNCTION(BlueprintCallable)
	float GetChargingTime() { return StrongAttackChargeTimer; }

	UFUNCTION(BlueprintCallable)
	void UpdateWeaponMesh(int32 WeaponID, bool bLeftHandWeapon);

	void SetAnimInstance(const FGameDS_HeroSkillSetDataTable* InSkillSet, const TMap<FString, UAnimMontage*>& InMontages);
	
	void OnOpenInventory();
	void OnOpenMenu();
	void OnOpenStatus();
	void OnOpenSoulShelter();
	void OnDrinkPotionAnim();
	void DrinkPotion();
	bool SetHeroAction(EGameDS_HeroActionState InAction);
	int32 GetSpawnPoint() const;
	void SetSpawnPoint(int32 InSpawnPoint) { SpawnPoint = InSpawnPoint; }

	UGameDS_InventoryController* GetInventoryController() const { return InventoryController; }
	UGameDS_LockOnController* GetLockOnController() const { return LockOnController; }
	UGameDS_HeroCharacterAnimInstance* GetHeroAnimInstance() const { return AnimInstance; }

	bool GetMoving() const { return bMoving; }
	void SetMoving(bool InMoving) { bMoving = InMoving;}
	bool GetRunning() const { return bRunning; }
	bool GetEvading();
	bool GetDefesning() const { return bDefensing; }
	void SetAttacking(bool InAttack) {bAttacking = InAttack;}
	void SetHeroNormalState();

	UFUNCTION(BlueprintCallable)
	bool GetAttacking() {return bAttacking;}

	UPROPERTY(BlueprintReadWrite)
	bool bForceEvadable = false;
	
	bool bDefensing = false;
	bool bAttacking = false;
	bool bMoving = false;
	bool bPressedInput = false;
	bool bRunning = false;
	bool bInteractionArea = false;
	bool bInteraction = false;
	bool bAttackCharging = false;
	UPROPERTY(VisibleAnywhere, Category = Character)
	bool bJumping = false;
	int32 LastInteractionID = INDEX_NONE;
	float UpDownValue;
	float LeftRightValue;
	double InputPressedStartTime;
	double InputElapsedTime;
	float JumpForwardVelocity;
	float StrongAttackChargeTimer;
	float MoveTimer;
	float LockOnTargetChangeTimer;
	void DefenseAttack();
	void EndDefenseAttack();

	void OnAttackEnd();
	void SetActorRotationWithForce(FRotator InRotator);
private:
	void CameraInit();
	void CharacterMeshInit();
	void SetHeroRotation(bool bBounded);
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UGameDS_InventoryController* InventoryController;
#pragma region  Input

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UGameDS_InputController* InputController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TMap<EGameDS_HeroActionType, UInputAction*> ActionMap;

#pragma endregion


#pragma region Camera & Lock On

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UGameDS_LockOnController* LockOnController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	UGameDS_HeroCharacterAnimInstance* AnimInstance;

	UPROPERTY()
	FName WeaponSocketName = TEXT("weapon_r");
	
	UPROPERTY(VisibleAnywhere, Category = Character)
	int32 AttackIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = Character)
	int32 AttackIndexMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LeftHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RightHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	int32 CurrentWeaponID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	FVector2D SavedMovementVector;
	
	//test
	bool bLastTick = true;

	int32 SpawnPoint = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	EGameDS_HeroNextActionState HeroNextState;
	
	FRotator HeroNextRotator = FRotator::ZeroRotator;

	bool bUseAllStamina = false;
	bool bPressedSpace = false;
	bool bReleaseSpace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	EGameDS_GenderType GenderType;
};
