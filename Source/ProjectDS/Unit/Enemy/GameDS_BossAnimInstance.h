// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectDS/Unit/Enemy/GameDS_EnemyAnimInstance.h"
#include "GameDS_BossAnimInstance.generated.h"

class AGameDS_Boss;

USTRUCT(BlueprintType)
struct FGameDS_BossAnimationAliasData : public FGameDS_AnimationAliasData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 Phase;
};

USTRUCT(BlueprintType)
struct FGameDS_AnimationAliasDataArray
{
	GENERATED_BODY()

	TArray<FGameDS_AnimationAliasData> AliasDataArray;
};

UCLASS()
class UGameDS_BossAnimInstance : public UGameDS_EnemyAnimInstance
{
	GENERATED_BODY()
public:
	UGameDS_BossAnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void InitAnimationAlias() override;
	virtual void EndDieAnimCallBack() override;
	
	void SetRun(bool InRun) { bRun = InRun; }
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bRun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	TArray<FGameDS_BossAnimationAliasData> BossAnimAlias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	TMap<int32, FGameDS_AnimationAliasDataArray> BossAnimAliasMap;

	UPROPERTY()
	AGameDS_Boss* OwnerBoss;
};
