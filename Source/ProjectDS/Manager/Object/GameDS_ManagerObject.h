#pragma once
#include "GameDS_ManagerObject.generated.h"

UCLASS()
class UGameDS_ManagerObject : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	virtual void Tick(float DeltaTime, UGameInstance* InGameInstance);
};
