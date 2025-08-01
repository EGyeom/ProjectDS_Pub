#pragma once
#include "Engine/GameInstance.h"
#include "GameDSInstance.generated.h"

UCLASS()
class PROJECTDS_API UGameDSInstance final : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UGameDSInstance();
	virtual void Init() override;
	virtual void Shutdown() override;
};
