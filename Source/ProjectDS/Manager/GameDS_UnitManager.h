#pragma once
#include "Engine/GameInstance.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/GameDS_UnitDefine.h"
#include "GameDS_UnitManager.generated.h"

class AGameDS_Summon;
class AGameDS_Character;
UCLASS()
class UGameDS_UnitManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	template<typename CharacterClass>
	CharacterClass* GetCharacter(int32 UnitID);
	AGameDS_Character* GetCharacter(int32 UnitID);
	
	template<typename CharacterClass>
	CharacterClass* GetLocalCharacter();
	AGameDS_Character* GetLocalCharacter();

	template<typename CharacterClass>
	void GetAllCharacter(TArray<CharacterClass*>& CharacterList);
	void GetAllCharacter(EGameDS_UnitType InUnitType, TArray<AGameDS_Character*>& CharacterList);

	UFUNCTION(BlueprintCallable)
	int32 GetNextUnitID() { return ++LastUnitID; }
	
	int32 GetNextSummonUnitID() { return ++LastSummonUnitID; }
	int32 GetLocalID() const { return LocalID; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetBossID() const { return BossID; }
	UFUNCTION(BlueprintCallable)
	int32 GetLastInteractionID();
	UFUNCTION(BlueprintCallable)
	int32 SpawnCharacter(const FGameDS_UnitSpawnOption& InSpawnOption);
	
	void DespawnCharacter(int32 UnitID);
	int32 SpawnSummonObject(const FGameDS_SummonUnitSpawnOption& InSummonSpawnOption);

	void RespawnAllUnit();

	int32 GetSummonOwnerUnitID(int32 InSummonUnitID);
	
	float GetDistBetweenUnit(int32 FromUnitID, int32 ToUnitID);
	float GetDistFromUser(int32 ToUnitID);

private:
	int32 LastUnitID = INDEX_NONE;
	int32 LastSummonUnitID = INDEX_NONE;
	
	int32 LocalID = INDEX_NONE;
	int32 BossID = INDEX_NONE;
	UPROPERTY()
	TMap<int32, AGameDS_Character*> CharacterMap;
	UPROPERTY()
	TMap<int32, AGameDS_Summon*> SummonMap;
	
	TMap<int32, FGameDS_UnitSpawnOption> AllSpawnDataList;
};

template <typename CharacterClass>
CharacterClass* UGameDS_UnitManager::GetCharacter(int32 UnitID)
{
	if (UnitID == INDEX_NONE)
		return nullptr;

	AGameDS_Character* Character = nullptr;
	if (CharacterMap.Find(UnitID))
		Character = *CharacterMap.Find(UnitID);

	if (Character == nullptr)
		return nullptr;

	return GameDS_CastCharacter<CharacterClass>(Character);
}

template <typename CharacterClass>
CharacterClass* UGameDS_UnitManager::GetLocalCharacter()
{
	return GetCharacter<CharacterClass>(LocalID);
}

template <typename CharacterClass>
void UGameDS_UnitManager::GetAllCharacter(TArray<CharacterClass*>& CharacterList)
{
	for (auto UnitData : CharacterMap)
	{
		if (UnitData.Value == nullptr)
			continue;

		if (auto CastCharacter = Cast<CharacterClass>(UnitData.Value))
			CharacterList.Add(CastCharacter);
	}
}
