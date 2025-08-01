#pragma once
#include "Animation/AnimInstance.h"
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GameDS_UnitDefine.h"
#include "GameDS_UnitAnimInstance.generated.h"

class AGameDS_Character;
enum class EGameDS_UnitAnimType : uint8;

USTRUCT(BlueprintType)
struct FGameDS_AnimationAliasData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EGameDS_UnitAnimType AnimType;

	UPROPERTY(EditAnywhere)
	int32 SkillID;
};

UCLASS()
class UGameDS_UnitAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UGameDS_UnitAnimInstance();
	
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	FSimpleDelegate EndAttackDelegate;
	FSimpleDelegate EndHitDelegate;
	FSimpleDelegate EndEvadeDelegate;
	FSimpleDelegate EndModeChangeDelegate;

	virtual void InitAnimationAlias();
	const FGameDS_SkillDataTable* GetSkillData(EGameDS_UnitAnimType InAnimType, int32 Index = 0);
	const TArray<int32>& GetAllSkillID() const { return SkillIDList; }
	int32 GetSkillID(EGameDS_UnitAnimType InAnimType, int32 Index = 0);
	void GetSkillID(EGameDS_UnitAnimType InAnimType, TArray<int32>& SkillIDListRef);
	void PlayMontage(EGameDS_UnitAnimType InAnimType, int32 Index = 0);
	void PlayMontage(int32 SkillID);
	void PlayCCMotion(EGameDS_CrowdControlType InCCType);

	void ChangeBattleMode(bool bActive);

	UFUNCTION(BlueprintCallable)
	void EndAttackNotify();
	UFUNCTION(BlueprintCallable)
	void EndHitNotify();
	UFUNCTION(BlueprintCallable)
	void EndEvadeNotify();
	UFUNCTION(BlueprintCallable)
	void EndModeChangeNotify();
	UFUNCTION(BlueprintCallable)
	bool SpawnSkillCollision(int32 InSkillInfoID, bool bNeutral);
	UFUNCTION(BlueprintCallable)
	bool SpawnSkillCollisionWithCharging(int32 InSkillInfoID, bool bNeutral, float InChargingTime);
	UFUNCTION(BlueprintCallable)
	void SpawnSummonActor(int32 InSummonID, FName SocketName);
	UFUNCTION(BlueprintCallable)
	void SpawnSummonActorWithAngle(int32 InSummonID, float InAngle, FName SocketName);
	
	UFUNCTION(BlueprintCallable)
	virtual void EndDieAnimCallBack();
	UFUNCTION(BlueprintCallable)
	void AddStatusEffectNotify(int32 InDataID, float InDuration);
	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetGroundPhysMaterial(EGameDS_FootStepType InFootStepType, FVector& OutLocation);

	void GetAnimationAlias(TArray<FGameDS_AnimationAliasData>& AliasList) { AliasList = AnimationAlias; }
	void CachingAliasData(int32 SkillID, FGameDS_SkillDataTable* SkillData);
	UAnimMontage* GetMontageBySkillID(int32 SkillID);
	UFUNCTION(BlueprintCallable)
	bool IsLookAt() const { return bLookAt; }
	UFUNCTION(BlueprintCallable)
	bool IsLeftMove() const;
	UFUNCTION(BlueprintCallable)
	bool IsBattle() const { return bBattleMode; };
	
	bool GetTurnState() { return bTurn; }
	bool GetTurnLeft() { return bLeftTurn; }

	void SetTurnState(bool bActive) { bTurn = bActive; }
	void SetTurnLeft(bool bLeft) { bLeftTurn = bLeft; }
	void SetLookAt(bool bActive) { bLookAt = bActive; }
	void SetHit(bool bActive) { bHit = bActive; }
	void SetSummonScaleValue(float bValue) { SummonScaleValue = bValue;}
	void SetEvade(bool InEvade) { bEvade = InEvade; }

protected:
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	AGameDS_Character* OwnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	TArray<FGameDS_AnimationAliasData> AnimationAlias; 

	void ResetAliasData();
private:
	bool IsValidSkillData(EGameDS_UnitAnimType InAnimType, int32 Index = 0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bTurn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bLeftTurn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bLookAt;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bMoveLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bBattleMode;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bHit;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	float SpeedValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	FVector2D Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	float SummonScaleValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Unit, Meta = (AllowPrivateAccess = true))
	bool bEvade;

	UPROPERTY(EditAnywhere, Category = "CCMotion")
	UAnimMontage* NormalKnockBackMotion;

	UPROPERTY(EditAnywhere, Category = "CCMotion")
	UAnimMontage* HeavyKnockBackMotion;

	UPROPERTY()
	TMap<int32, FGameDS_SkillDataTable> AliasCachingMap;
	
	TArray<int32> SkillIDList;
};
