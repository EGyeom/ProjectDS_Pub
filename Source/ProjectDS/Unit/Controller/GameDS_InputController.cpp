// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectDS/Unit/Controller/GameDS_InputController.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacter.h"
#include "ProjectDS/Unit/Hero/GameDS_HeroCharacterAnimInstance.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "ProjectDS/Manager/GameDS_UIManager.h"
#include "EnhancedActionKeyMapping.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGameDS_InputController::Init(AGameDS_HeroCharacter* InCharacter, UInputMappingContext* InputMapping, TMap<EGameDS_HeroActionType, UInputAction*> InActionMap)
{
	Character = InCharacter;
	Controller = Cast<APlayerController>(Character->Controller);
	DefaultMappingContext = InputMapping;
	ActionMap = InActionMap;

	if (Controller != nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
		if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(Controller->GetLocalPlayer()->GetGameInstance()))
		{
			UIManager->SetControl(InCharacter, Controller);
		}
	}
}

void UGameDS_InputController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ActionMap.Find(EGameDS_HeroActionType::Move) != nullptr)
		{
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Move), ETriggerEvent::Triggered, Character, &AGameDS_HeroCharacter::Move);
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Move), ETriggerEvent::Completed, Character, &AGameDS_HeroCharacter::MoveEnd);
		}
		
		if (ActionMap.Find(EGameDS_HeroActionType::Look) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Look), ETriggerEvent::Triggered, Character, &AGameDS_HeroCharacter::Look);
		
		if (ActionMap.Find(EGameDS_HeroActionType::Jump) != nullptr)
		{
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Jump), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::PreJump);
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Jump), ETriggerEvent::Completed, Character, &AGameDS_HeroCharacter::StopJumping);
		}
		
		if (ActionMap.Find(EGameDS_HeroActionType::NormalAttack) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::NormalAttack), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnAttack);
		
		if (ActionMap.Find(EGameDS_HeroActionType::SpecialAttack) != nullptr)
		{
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::SpecialAttack), ETriggerEvent::Triggered, Character, &AGameDS_HeroCharacter::OnStrongAttack);
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::SpecialAttack), ETriggerEvent::Completed, Character, &AGameDS_HeroCharacter::OnStrongAttackReleased);
		}

		if (ActionMap.Find(EGameDS_HeroActionType::Defense) != nullptr)
		{
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Defense), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnDefense);
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Defense), ETriggerEvent::Completed, Character, &AGameDS_HeroCharacter::OnDefenseEnd);
		}
		
		if (ActionMap.Find(EGameDS_HeroActionType::Evade) != nullptr)
		{
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Evade), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnPressedSpace);
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Evade), ETriggerEvent::Completed, Character, &AGameDS_HeroCharacter::OnReleasedSpace);
		}

		if (ActionMap.Find(EGameDS_HeroActionType::Menu) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Menu), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnOpenMenu);
		
		if (ActionMap.Find(EGameDS_HeroActionType::Inventory) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Inventory), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnOpenInventory);

		if (ActionMap.Find(EGameDS_HeroActionType::Status) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Status), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnOpenStatus);

		if (ActionMap.Find(EGameDS_HeroActionType::ChangeItem) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::ChangeItem), ETriggerEvent::Started, this, &UGameDS_InputController::ArrowKeyInput);

		if (ActionMap.Find(EGameDS_HeroActionType::DrinkPotion) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::DrinkPotion), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnDrinkPotionAnim);

		if (ActionMap.Find(EGameDS_HeroActionType::Interaction) != nullptr)
			EnhancedInputComponent->BindAction(*ActionMap.Find(EGameDS_HeroActionType::Interaction), ETriggerEvent::Started, Character, &AGameDS_HeroCharacter::OnOpenSoulShelter);
	}
}

void UGameDS_InputController::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	Character->AddControllerYawInput(LookAxisVector.X);
	Character->AddControllerPitchInput(LookAxisVector.Y);
}

void UGameDS_InputController::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	FRotator Rotation;

	if (Character->GetCharacterMovement()->IsFalling() == false)
	{
		 Rotation = Controller->GetControlRotation();
	}
	else
	{
		 Rotation = Character->GetActorRotation();
	}
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	Character->AddMovementInput(ForwardDirection, MovementVector.Y);
	Character->AddMovementInput(RightDirection, MovementVector.X);
}

void UGameDS_InputController::ArrowKeyInput(const FInputActionValue& Value)
{
	const FVector2D Keyinput = Value.Get<FVector2D>();
	EGameDS_ArrowKey Arrow = EGameDS_ArrowKey::Left;

	if (Keyinput.X > 0)
	{
		Arrow = EGameDS_ArrowKey::Right;
	}
	else if (Keyinput.Y > 0)
	{
		Arrow = EGameDS_ArrowKey::Up;
	}
	else if (Keyinput.Y < 0)
	{
		Arrow = EGameDS_ArrowKey::Down;
	}

	Character->OnChangeItem(Arrow);
}
