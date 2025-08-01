#pragma once
#include "GameFramework/PlayerStart.h"
#include "GameDS_HeroSpawnPoint.generated.h"

UCLASS()
class AGameDS_HeroSpawnPoint : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpawnPoint = INDEX_NONE;
};
