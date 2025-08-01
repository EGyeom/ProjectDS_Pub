#pragma once
#include "ProjectDS/Manager/GameDS_CustomDatas.h"
#include "GameDS_CrowdControlController.generated.h"

enum class EGameDS_BlockType : uint8;
class AGameDS_Character;

USTRUCT()
struct FGameDS_CrowdControlDetailData
{
	GENERATED_BODY()

	FGameDS_CrowdControlData CrowdControlData;
	FVector DestLocation;
	float InterpSpeed = 0.0f;
};

UCLASS()
class UGameDS_CrowdControlController : public UObject
{
	GENERATED_BODY()
public:
	void Init(AGameDS_Character* InOwnerCharacter);
	void Tick(float DeltaTime);

	void ApplyCrowdControl(const TArray<int32>& InCrowdControlIDList, AActor* Caster, FVector CollisionSpawnPoint);
	void UpdateCrowdControl(float DeltaTime);
	void ApplyPushBack(const FGameDS_CrowdControlData& InCCData);
	bool CheckBlockType(EGameDS_BlockType InBlockType) const;
	void ChangeBlockState(EGameDS_BlockType InBlockType, bool bFlag);
	void RemoveAllCrowdControl();
	bool IsPlayingCCMotion();

private:
	UPROPERTY()
	AGameDS_Character* OwnerCharacter;

	TArray<FGameDS_CrowdControlDetailData> CrowdControlList;
	bool bMoveBlock = false;
	bool bActionBlock = false;
	bool bRotateBlock = false;
};
