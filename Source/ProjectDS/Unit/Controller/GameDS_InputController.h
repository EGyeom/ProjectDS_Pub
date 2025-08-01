// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "GameDS_InputController.generated.h"

class AGameDS_HeroCharacter;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
struct FEnhancedActionKeyMapping;

UCLASS()
class PROJECTDS_API UGameDS_InputController : public UObject
{
	GENERATED_BODY()
public:
	void Init(AGameDS_HeroCharacter* InCharacter, UInputMappingContext* InputMapping, TMap<EGameDS_HeroActionType, UInputAction*> InActionMap);
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void ArrowKeyInput(const FInputActionValue& Value);
private:
	AGameDS_HeroCharacter* Character;
	APlayerController* Controller;
	UInputMappingContext* DefaultMappingContext;
	TMap<EGameDS_HeroActionType, UInputAction*> ActionMap;
};
