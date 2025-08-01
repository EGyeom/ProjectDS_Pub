#pragma once
#include "Tickable.h"
#include "GameDS_GameManager.generated.h"

class UGameDS_ManagerObject;

UCLASS()
class UGameDS_GameManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	template <typename ClassType>
	ClassType* GetManagerObject() const;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	void RequestRespawnAllUnit(float InDelay = 2.0f);

	// Command Function
	void ExecuteConsoleCommand(const TArray<FString>& Arg);

private:
	UPROPERTY()
	TArray<UGameDS_ManagerObject*> ManagingObjectList;

	bool bIsCreateOnRunning = false;
	bool bRequestRespawn = false;
	float RespawnRemainTime = 0.0f;
};

template <typename ClassType>
ClassType* UGameDS_GameManager::GetManagerObject() const
{
	for (UGameDS_ManagerObject* ManagerObject : ManagingObjectList)
	{
		if (ClassType* Manager = Cast<ClassType>(ManagerObject))
			return Manager;
	}
	
	return nullptr;
}
