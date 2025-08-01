#pragma once
#include "Object/GameDS_ManagerObject.h"
#include "GameDS_SummonManager.generated.h"

class AGameDS_Summon;
class AGameDS_Character;

USTRUCT()
struct FGameDS_SummonArrayStruct
{
	GENERATED_BODY()
	FGameDS_SummonArrayStruct()
	{

	}

	FGameDS_SummonArrayStruct(AGameDS_Summon* InSummon)
	{
		SummonArray.Add(InSummon);
	}

	TArray<AGameDS_Summon*> SummonArray;
};

UCLASS()
class UGameDS_SummonManager : public UGameDS_ManagerObject
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime, UGameInstance* InGameInstance) override;

	void AddSummon(AGameDS_Summon* InSummon);
	void RemoveExpiredSummon(UGameInstance* InGameInstance);
	void RemoveSummonCallBack(AGameDS_Summon* InSummon);

	AGameDS_Summon* MultiSpawnSummonNearbyCharacter(AGameDS_Character* Target, int32 SkillID);

private:
	UPROPERTY()
	TArray<AGameDS_Summon*> SummonList;
	UPROPERTY()
	TArray<AGameDS_Summon*> ExpiredSummonList;
	UPROPERTY()
	TMap<int32, FGameDS_SummonArrayStruct> SummonListBySkillID;
};
