#pragma once
#include "GameDS_StatusEffectController.generated.h"

class AGameDS_Character;
enum class EGameDS_StatusEffectType : uint8;

struct FGameDS_StatusEffectInfo
{
	int32 DataID = INDEX_NONE;
	float RemainTime = INDEX_NONE;
};

UCLASS()
class UGameDS_StatusEffectController : public UObject
{
	GENERATED_BODY()
public:
	void Init(AGameDS_Character* InOwnerCharacter);
	void Tick(float DeltaTime);
	
	void AddStatusEffect(int32 InDataID, float InDuration = INDEX_NONE);
	bool IsDamageable() const;

private:
	bool IsActiveType(EGameDS_StatusEffectType InStatusEffectType) const;
	TMap<EGameDS_StatusEffectType, TArray<FGameDS_StatusEffectInfo>> StatusEffectMap;
	
	UPROPERTY()
	AGameDS_Character* OwnerCharacter;
};
