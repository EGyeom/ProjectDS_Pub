#include "GameDS_HeroCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameDS_HeroCharacterAnimInstance.h"
#include "ProjectDS/GameDSInstance.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/ProjectDS.h"
#include "ProjectDS/Util/GameDS_EnumUtil.h"
#include "ProjectDS/Unit/Controller/GameDS_LockOnController.h"
#include "ProjectDS/Util/GameDS_Util.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "ProjectDS/Unit/Controller/GameDS_CrowdControlController.h"
#include "ProjectDS/Unit/Controller/GameDS_InputController.h"
#include "ProjectDS/Unit/Controller/GameDS_InventoryController.h"
#include "ProjectDS/Unit/Controller/GameDS_StatController.h"

AGameDS_HeroCharacter::AGameDS_HeroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	RightHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RIGHTHAND"));
	LeftHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LEFTHAND"));
	CameraInit();
	CharacterMeshInit();
	InputPressedStartTime = -1;
	InputElapsedTime = -1;
	MoveTimer = -1.f;
	SavedMovementVector = FVector2D::ZeroVector;
	HeroNextState = EGameDS_HeroNextActionState::None;
	HeroNextRotator = FRotator::ZeroRotator;
}

void AGameDS_HeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	LockOnController = NewObject<UGameDS_LockOnController>();
	InventoryController = NewObject<UGameDS_InventoryController>();

	InventoryController->InitController(this);

	LoadData();
}

void AGameDS_HeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGameDS_HeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bLastTick)
	{
		bLastTick = false;
		
		if (UGameDS_UIManager* UIManager = FGameInstanceHolder::GetSubSystem<UGameDS_UIManager>())
			UIManager->UpdateInventory();
	}
	
	LockOnController->Update(DeltaTime);

	if (bUseAllStamina == true && GetStatController()->GetStaminaRestoreType() != EGameDS_StaminaRestoreType::Exhaustion)
	{
		bUseAllStamina = false;
	}

	// Jump 관련
	if (bJumping)
	{
		AddMovementInput(GetActorForwardVector(), JumpForwardVelocity);
	}

	if (GetMovementComponent()->IsMovingOnGround() == true)
	{
		bJumping = false;
		GetCharacterMovement()->JumpZVelocity = PLAYER_DEFAULT_JUMP_HEIGHT;
	}

	// running
	if (InputPressedStartTime > 0.f)
	{
		InputElapsedTime = FPlatformTime::Seconds() - InputPressedStartTime;
		
		if (InputElapsedTime > 0.5f)
		{
			if (bMoving == true)
			{
				GetCharacterMovement()->MaxWalkSpeed = PLAYER_RUNSPEED;
				bRunning = true;
				//SetHeroAction(EGameDS_HeroActionState::Run);
				InputPressedStartTime = -1.f;
				InputElapsedTime = -1.f;
			}
		}
	}

	if (bReleaseSpace == true && bRunning == true)
	{
		bRunning = false;
		SetHeroNormalState();
	}

	if (HeroNextState != EGameDS_HeroNextActionState::None)
	{
		if (AnimInstance->GetMoveable() == true)
		{
			switch (HeroNextState)
			{
				case EGameDS_HeroNextActionState::Attack:
					OnAttack();
					break;
				case EGameDS_HeroNextActionState::Evade:
					OnReleasedSpace();
					break;
				case EGameDS_HeroNextActionState::Run:
 					OnPressedSpace();
					break;
			}
		HeroNextState = EGameDS_HeroNextActionState::None;
		}
	}

	if (bAttackCharging == true) // Charging
	{
		StrongAttackChargeTimer += DeltaTime;

		if (StrongAttackChargeTimer >= 2.0f)
		{
			OnStrongAttackReleased();
		}
	}

	if (bMoving == true && MoveTimer >= 0.f)
	{
		MoveTimer += DeltaTime;

		if (MoveTimer >= 2.0f && bAttacking == false)
		{
			AttackIndex = 0;
			MoveTimer = -1.f;
		}
	}

	if (HeroNextRotator.IsNearlyZero() == false)
	{
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), HeroNextRotator, DeltaTime, 10.f));

		if (FMath::IsNearlyEqual(GetActorRotation().Yaw, HeroNextRotator.Yaw))
			HeroNextRotator = FRotator::ZeroRotator;
	}
	
	TArray<EGameDS_StatResult> StatResultList;
	GetStatController()->Tick(DeltaTime, StatResultList);

	if (StatResultList.Contains(EGameDS_StatResult::ZeroStamina) == true)
	{
		GetStatController()->SetAction(EGameDS_HeroActionState::None);
		bUseAllStamina = true;
	}

	if (LockOnController->IsLockOn() == true)
	{
		LockOnTargetChangeTimer += DeltaTime;
	}
}

void AGameDS_HeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputController = NewObject<UGameDS_InputController>();
	
	if (ActionMap.Num() != 0)
	{
		InputController->Init(this, DefaultMappingContext, ActionMap);
		InputController->SetupPlayerInputComponent(PlayerInputComponent);
	}
}

void AGameDS_HeroCharacter::SpawnInit(const FGameDS_UnitSpawnOption& InSpawnOption, const FGameDS_UnitStatInfo& InUnitStatInfo, const FGameDS_HeroStatInfo& InHeroStatInfo)
{
	Super::SpawnInit(InSpawnOption, InUnitStatInfo, InHeroStatInfo);

	GenderType = EGameDS_GenderType::Male;
	if (UGameDS_DataManager* DataManager = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>())
	{
		bool bMale = true;
		DataManager->LoadDataWithBool(STAT_INI_SECTION, GENDERINFO_INI_KEY, bMale);
		GenderType = bMale ? EGameDS_GenderType::Male : EGameDS_GenderType::Female;
	}

	SpawnPoint = InSpawnOption.SpawnPoint;

	SpawnDelegate.Broadcast();

	if (LockOnController != nullptr)
	{
		LockOnController->Init(this, SpringArm, Camera);
	}
}

void AGameDS_HeroCharacter::OnDamaged(const FGameDS_DamageInfo& InDamageInfo)
{
	if (AnimInstance == nullptr)
		return;

	if (AnimInstance->GetEvading() || IsDied() == true)
		return;

	if (bDefensing == true)
		return;

	Super::OnDamaged(InDamageInfo);

	UpdateInventory();
}

bool AGameDS_HeroCharacter::IsUnitType(EGameDS_UnitType InUnitType)
{
	if (InUnitType == EGameDS_UnitType::Hero)
		return true;
	
	return Super::IsUnitType(InUnitType);
}

void AGameDS_HeroCharacter::LoadData()
{
	if (UGameDS_DataManager* DataManager = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>())
	{
		FGameDS_HeroCreateInfo CreateInfo;
		DataManager->LoadHeroData(CreateInfo);

		if (UGameDS_StatController* HeroStatController = GetStatController())
			HeroStatController->LoadStat(CreateInfo);

		if (UGameDS_InventoryController* HeroInventoryController = GetInventoryController())
			HeroInventoryController->LoadItem(CreateInfo);
	}
}

void AGameDS_HeroCharacter::UpdateInventory()
{
	if (InventoryController == nullptr)
		return;

	// ItemMesh Update
	int32 LWeaponID = InventoryController->GetMainWeapon(true) == nullptr ? INDEX_NONE : InventoryController->GetMainWeapon(true)->DataID;
	int32 RWeaponID = InventoryController->GetMainWeapon(false) == nullptr ? INDEX_NONE : InventoryController->GetMainWeapon(false)->DataID;

	UpdateWeaponMesh(LWeaponID, true);
	UpdateWeaponMesh(RWeaponID, false);

	if (InventoryUpdateDelegate.IsBound())
		InventoryUpdateDelegate.Broadcast(InventoryController);
}

void AGameDS_HeroCharacter::InitInteractionSetting(bool bActive)
{
	bInteraction = bActive;
}

void AGameDS_HeroCharacter::Look(const FInputActionValue& Value)
{
	if (LockOnController->IsLockOn() == true)
	{
		if (LockOnTargetChangeTimer < 1.0f)
			return;
		
		float XAxisValue = Value.Get<FVector2D>().X;
		
		UE_LOG(LogTemp, Log, TEXT("%f"), XAxisValue);

		if (XAxisValue >= 10.f)
		{
			LockOnController->LockOn(true, false);
		}
		else if (XAxisValue <= -10.f)
		{
			LockOnController->LockOn(true, true);
		}
		else return;

		LockOnTargetChangeTimer = 0.f;
	}

	if (bUseControllerRotationYaw == false && Controller != nullptr)
	{
		InputController->Look(Value);
	}
}

void AGameDS_HeroCharacter::Move(const FInputActionValue& Value)
{
	if (IsInteraction() == true || AnimInstance == nullptr || IsDied() == true 
		|| GetMovementComponent()->IsMovingOnGround() == false || GetCrowdControlController()->IsPlayingCCMotion() == true)
		return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	SavedMovementVector = MovementVector;

	if (GetEvading() == false)
	{
		LeftRightValue = MovementVector.X;
		UpDownValue = MovementVector.Y;
	}

	bPressedInput = MovementVector.IsZero() == false;
	
	if (bAttacking == false)
	{
		HeroNextRotator = GetActorRotation();
	}

	if (AnimInstance->GetMoveable())
	{
		if (bJumping == false)
		{
			if (bRunning == false)
				SetHeroAction(EGameDS_HeroActionState::None);
			
			bMoving = true;
			bAttacking = false;
			
			if (MoveTimer < 0.f)
				MoveTimer = 0.f;
			
			AnimInstance->StopAllMontages(0.2f);
			InputController->Move(Value);
		}
	}
}

void AGameDS_HeroCharacter::MoveEnd()
{
	bMoving = false;
}

void AGameDS_HeroCharacter::ToggleTargetLock()
{
	if (LockOnController->GetTarget() != nullptr)
	{
		LockOnController->LockOff();
		return;
	}

	LockOnController->LockOn(false, false);
}

void AGameDS_HeroCharacter::OnInteraction(bool bEnable, int32 InInteractionID/* = INDEX_NONE*/)
{
	bInteractionArea = bEnable;
	LastInteractionID = InInteractionID;
}

void AGameDS_HeroCharacter::PreJump()
{
	if (bJumping == true || IsDied() == true || AnimInstance->GetMoveable() == false)
		return;
	bPressedJump = true;
	
	JumpForwardVelocity = FMath::Abs(FVector::DotProduct(GetVelocity(), GetActorForwardVector()));

	float CurrentSpeed = (JumpForwardVelocity - PLAYER_WALKSPEED) < 0.f ? 0.f : JumpForwardVelocity - PLAYER_WALKSPEED;
	GetCharacterMovement()->JumpZVelocity = PLAYER_DEFAULT_JUMP_HEIGHT + CurrentSpeed * 0.2f;
	bJumping = true;
}

void AGameDS_HeroCharacter::Jump()
{
	Super::Jump();
}

void AGameDS_HeroCharacter::StopJumping()
{
	Super::StopJumping();
}

void AGameDS_HeroCharacter::OnAttack()
{
	if (AttackIndexMax == 0 || IsDied() == true)
		return;

	if (InventoryController == nullptr)
		return;

	if (InventoryController->IsValidMainItem(EGameDS_ItemInnerType::RWeapon) == false)
		return;

	if (GetCrowdControlController()->IsPlayingCCMotion() == true)
		return;

	if (!GetMovementComponent()->IsFalling() && bJumping == false && AnimInstance->GetMoveable() == true)
	{
		if (SetHeroAction(EGameDS_HeroActionState::Attack) == false)
			return;

		SetHeroRotation(true);
		bAttacking = true;
		AnimInstance->StopAllMontages(0.5f);
		AnimInstance->PlayAttackMontage();
		AnimInstance->JumpToSection("NormalAttack", AttackIndex);
		AttackIndex = (AttackIndex + 1) % AttackIndexMax;
	}
	else if (AnimInstance->GetMoveable() == false)
	{
		if (bAttackCharging == true)
			return;

		if (HeroNextState == EGameDS_HeroNextActionState::None)
		{
			HeroNextState = EGameDS_HeroNextActionState::Attack;
		}
	}
}

void AGameDS_HeroCharacter::OnStrongAttack()
{
	if (IsDied() == true)
		return;

	if (AnimInstance->GetMoveable() == true)
	{
		if (SetHeroAction(EGameDS_HeroActionState::StrongAttack) == false)
			return;
		bAttackCharging = true;
		StrongAttackChargeTimer = 0.f;
		AnimInstance->PlayStrongAttackMontage();
		AnimInstance->JumpToSection("StrongAttack", 0);
	}
}

void AGameDS_HeroCharacter::OnStrongAttackReleased()
{
	if (IsDied() == true)
		return;

	if (bAttackCharging == true)
	{
		AnimInstance->PlayStrongAttackMontage();
		AnimInstance->JumpToSection("StrongAttack", 1);
		SetHeroAction(EGameDS_HeroActionState::None);
		bAttackCharging = false;
	}
}

void AGameDS_HeroCharacter::OnDefense()
{
	if (bDefensing == true || IsDied() == true)
		return;
	bDefensing = true;
}

void AGameDS_HeroCharacter::OnDefenseEnd()
{
	bDefensing = false;
}

void AGameDS_HeroCharacter::OnPressedSpace()
{
	bPressedSpace = true;
	bReleaseSpace = false;

	if (IsDied() == true || GetMovementComponent()->IsMovingOnGround() == false)
		return;
	
	InputPressedStartTime = FPlatformTime::Seconds();
	/*
	if (AnimInstance->GetMoveable() == true && AnimInstance->GetEvading() == false)
	{
		InputPressedStartTime = FPlatformTime::Seconds();
	}
	else
	{
		if (HeroNextState == EGameDS_HeroNextActionState::None)
		{
			HeroNextState = EGameDS_HeroNextActionState::Run;
			OnAttackEnd();
		}
	}
	*/
}

void AGameDS_HeroCharacter::OnReleasedSpace()
{
	bPressedSpace = false;
	bReleaseSpace = true;

	if (AnimInstance == nullptr)
		return;
	
 	if (IsDied() == true)
		return;

	const bool bPlayingCCMotion = GetCrowdControlController()->IsPlayingCCMotion();
	if (!AnimInstance->GetMoveable() && bPlayingCCMotion)
	{
		if (bForceEvadable)
		{
			bForceEvadable = false;
			AnimInstance->ForceEvadeSetting();
			GetCrowdControlController()->RemoveAllCrowdControl();
		}
		SetHeroNormalState();
		return;
	}

	if (bRunning == true || bJumping == true || bUseAllStamina == true || AnimInstance->IsDrink())
	{
		bUseAllStamina = false;
		SetHeroNormalState();
		return;
	}
 	else if (AnimInstance->GetMoveable() && (AnimInstance->GetEvading() == false || HeroNextState == EGameDS_HeroNextActionState::Evade))
	{
 		if (SetHeroAction(EGameDS_HeroActionState::Evade) == false)
			return;

  		SetHeroRotation(false);
		AnimInstance->StopAllMontages(0.1f);
		AnimInstance->OnEvade();
		SetHeroNormalState();
		return;
	}
 	else if (AnimInstance->GetEvading() == false)
	{
  		HeroNextState = EGameDS_HeroNextActionState::Evade;
		OnAttackEnd();
		return;
	}
}

void AGameDS_HeroCharacter::OnEndEvade()
{
	GetStatController()->SetAction(EGameDS_HeroActionState::None);
}

void AGameDS_HeroCharacter::OnUpdateUI()
{
	if (UGameDS_UIManager* UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->UpdateUI(GetUnitID(), GetUnitType(), *GetStatController());
	}
}

void AGameDS_HeroCharacter::OnEndExhaustion()
{
	InputElapsedTime = INDEX_NONE;
	InputPressedStartTime = INDEX_NONE;
}

void AGameDS_HeroCharacter::OnChangeItem(const EGameDS_ArrowKey Arrow)
{
	switch (Arrow)
	{
	case EGameDS_ArrowKey::Left:
		{
			InventoryController->SwapItem(EGameDS_ItemType::Weapon, true);
		}
		break;
	case EGameDS_ArrowKey::Right:
		{
			InventoryController->SwapItem(EGameDS_ItemType::Weapon, false);	
		}
		break;
	case EGameDS_ArrowKey::Up:
		break;
	case EGameDS_ArrowKey::Down:
		{
			InventoryController->SwapItem(EGameDS_ItemType::Potion, true);
		}
		break;
	default:
		break;
	}
}

FName AGameDS_HeroCharacter::GetWeaponSocketName() const
{
	return WeaponSocketName;
}

void AGameDS_HeroCharacter::UpdateWeaponMesh(int32 WeaponID, bool bLeftHandWeapon)
{
	if (UGameDS_DataManager* DataManager = UGameInstance::GetSubsystem<UGameDS_DataManager>(GetGameInstance()))
	{
		if (const FGameDS_ItemDataTable* ItemRow = DataManager->GetDataInRow<FGameDS_ItemDataTable>(WeaponID))
		{
			UStaticMeshComponent* WeaponMesh = bLeftHandWeapon ? LeftHand : RightHand;

			WeaponMesh->SetStaticMesh(ItemRow->ItemMesh);

			if (FGameDS_WeaponDataTable* WeaponData = DataManager->GetWeaponData(WeaponID))
			{
				const UDataTable* HeroSkillSetData = DataManager->GetDataTable<FGameDS_HeroSkillSetDataTable>();

				FGameDS_HeroSkillSetDataTable* HeroSkillSet = HeroSkillSetData->FindRow<FGameDS_HeroSkillSetDataTable>(FName(FString::FromInt(WeaponID)), TEXT(""));

				if (bLeftHandWeapon == false)
				{
					if (HeroSkillSet == nullptr)
						return;

					SetAnimInstance(HeroSkillSet, WeaponData->Montages);
					AttackIndex = 0;

					if (WeaponData->Montages.IsEmpty())
						AttackIndexMax = 0;
					else
						AttackIndexMax = (*WeaponData->Montages.Find("NormalAttack"))->GetNumSections();
				}
				
				FName SocketName = *FGameDS_EnumUtil::GetEnumToString(WeaponData->SpawnType);
				WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
			}
		}
	}
}

void AGameDS_HeroCharacter::SetAnimInstance(const FGameDS_HeroSkillSetDataTable* InSkillSet, const TMap<FString, UAnimMontage*>& InMontages)
{
	AnimInstance = Cast<UGameDS_HeroCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	
	AnimInstance->SetAnimationAlias(InSkillSet);

	if (AnimInstance != nullptr)
	{
		AnimInstance->InitMontages(InMontages);
		
		AnimInstance->EndAttackDelegate.BindLambda([this]
			{
				OnAttackEnd();
			}
		);

		AnimInstance->EndEvadeDelegate.BindLambda([this]
		{
			AnimInstance->EndEvade();
		});
	}
}

void AGameDS_HeroCharacter::OnOpenInventory()
{
	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->SetVisibleHeroUI(EGameDS_HeroUIType::Equipment, true);
	}
}

void AGameDS_HeroCharacter::OnOpenMenu()
{
	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->SetVisibleHeroUI(EGameDS_HeroUIType::Menu, true);
	}
}

void AGameDS_HeroCharacter::OnOpenStatus()
{
	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->SetVisibleHeroUI(EGameDS_HeroUIType::Status, true);
	}
}

void AGameDS_HeroCharacter::OnOpenSoulShelter()
{
	if (IsInteractionArea() == false)
		return;

	GetStatController()->OnResetStatCondition();
	InitInteractionSetting(true);
	SetInteractionCamera(true);
	
	GetInventoryController()->ResetAllPotionCount();

	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
	{
		UIManager->SetVisibleHeroUI(EGameDS_HeroUIType::SoulShelter, true);
	}
}

void AGameDS_HeroCharacter::OnDrinkPotionAnim()
{
	if (AnimInstance == nullptr)
		return;

	if (InventoryController == nullptr)
		return;

	if (AnimInstance->GetMoveable() == false)
		return;

	const UGameDS_PotionItemInfo* MainPotion = InventoryController->GetMainPotion();
	if (MainPotion == nullptr)
		return;

	if (MainPotion->DrinkableCount == 0)
		return;
	
	AnimInstance->SetDrink(true);
}

void AGameDS_HeroCharacter::DrinkPotion()
{
	if (AnimInstance == nullptr)
		return;

	if (InventoryController == nullptr)
		return;

	const UGameDS_PotionItemInfo* MainPotion = InventoryController->GetMainPotion();
	if (MainPotion == nullptr)
		return;

	if (MainPotion->DrinkableCount == 0)
		return;

	if (GetStatController() == nullptr)
		return;

	// Heal
	TArray<EGameDS_StatResult> StatResultList;
	GetStatController()->OnHealed(MainPotion->HealValue, StatResultList);
	GetInventoryController()->UseMainPotion();

	UpdateStat();
}
 
bool AGameDS_HeroCharacter::SetHeroAction(EGameDS_HeroActionState InAction)
{
	if (InAction == EGameDS_HeroActionState::None)
	{
		GetStatController()->SetAction(InAction);
		return false;
	}
	TArray<EGameDS_StatResult> StatResult;
	return GetStatController()->OnUseStamina(InAction, StatResult);
}

int32 AGameDS_HeroCharacter::GetSpawnPoint() const
{
	return SpawnPoint; 
}

bool AGameDS_HeroCharacter::GetEvading()
{
	return AnimInstance->GetEvading();
}

void AGameDS_HeroCharacter::CameraInit()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = LOCKOFF_SPRINGARM_LENGTH;
	SpringArm->SetRelativeRotation(FRotator(-75.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = true; 

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
}

void AGameDS_HeroCharacter::CharacterMeshInit()
{
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -93.f), FRotator(0.f, -90.f, 0.f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterSkeletalMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/G2_Warriors_01/Meshes/Characters/Combines/SK_Arthur_A1.SK_Arthur_A1'"));
	
	if (CharacterSkeletalMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(CharacterSkeletalMesh.Object);
	}
}

void AGameDS_HeroCharacter::OnAttackEnd()
{
	AttackIndex = 0;
	HeroNextRotator = FRotator::ZeroRotator;
	bAttacking = false;
	GetStatController()->SetAction(EGameDS_HeroActionState::None);
}

void AGameDS_HeroCharacter::SetHeroRotation(bool bBounded)
{
	HeroNextRotator = FRotator::ZeroRotator;
	HeroNextRotator.Yaw = GetControlRotation().Yaw;

	if (SavedMovementVector.X > 0.1f)
	{
		if (SavedMovementVector.Y > 0.1f) 
		{
			HeroNextRotator += FRotator(0.0f, 45.0f, 0.0f);
		}
		else if (SavedMovementVector.Y < -0.1f)
		{
			HeroNextRotator += FRotator(0.0f, 135.0f, 0.0f);
		}
		else
		{
			HeroNextRotator += FRotator(0.0f, 90.f, 0.0f);
		}
	}
	else if (SavedMovementVector.X < -0.1f)
	{
		if (SavedMovementVector.Y > 0.1f)
		{
			HeroNextRotator += FRotator(0.0f, -45.0f, 0.0f);
		}
		else if (SavedMovementVector.Y < -0.1f)
		{
			HeroNextRotator += FRotator(0.0f, 225.0f, 0.0f);
		}
		else
		{
			HeroNextRotator += FRotator(0.0f, -90.0f, 0.0f);
		}
	}
	else
	{
		if (SavedMovementVector.Y < -0.1f)
		{
			HeroNextRotator += FRotator(0.0f, 180.f, 0.0f);
		}
	}

	if (bBounded == true)
	{
		FVector CharacterForwardVec = GetActorForwardVector().GetSafeNormal2D();
		FVector InputForwardVec = HeroNextRotator.Vector().GetSafeNormal2D();

		float MaxAngle = 120.0f;

		float DotProductValue = FVector::DotProduct(CharacterForwardVec, InputForwardVec);
		FVector CrossResult = FVector::CrossProduct(CharacterForwardVec, InputForwardVec);
		float Angle = FMath::Acos(DotProductValue);
		float AngleInDegrees = FMath::RadiansToDegrees(Angle);

		if (FMath::Abs(AngleInDegrees) > MaxAngle)
		{
			FVector RotationAxis(0.0f, 0.0f, 1.0f);

			if (CrossResult.Z < 0.f)
			{
				MaxAngle *= -1;
			}
			FVector RotatedVector = CharacterForwardVec.RotateAngleAxis(MaxAngle, RotationAxis);

			HeroNextRotator = RotatedVector.Rotation();
		}
	}
	else
	{
		SetActorRotation(HeroNextRotator);
	}
	SavedMovementVector = FVector2D::ZeroVector;
}

void AGameDS_HeroCharacter::SetActorRotationWithForce(FRotator InRotator)
{
	HeroNextRotator = FRotator::ZeroRotator;

	SetActorRotation(InRotator);
}

void AGameDS_HeroCharacter::DefenseAttack()
{
	TArray<EGameDS_StatResult> Result;
	GetStatController()->OnUseStamina(EGameDS_HeroActionState::Defense, Result);
	GetStatController()->OnDamaged(3.0f, Result);

	if (Result.Contains(EGameDS_StatResult::ZeroHealth))
	{
		SetDie(true);

		if (OnDeadDelegate.IsBound())
			OnDeadDelegate.Broadcast();

	}
}

void AGameDS_HeroCharacter::SetHeroNormalState()
{
	bRunning = false;
	SetHeroAction(EGameDS_HeroActionState::None);
	HeroNextState = EGameDS_HeroNextActionState::None;
	GetCharacterMovement()->MaxWalkSpeed = PLAYER_WALKSPEED;
	InputElapsedTime = -1;
	InputPressedStartTime = -1;
}

void AGameDS_HeroCharacter::EndDefenseAttack()
{
	GetStatController()->SetAction(EGameDS_HeroActionState::None);
}
