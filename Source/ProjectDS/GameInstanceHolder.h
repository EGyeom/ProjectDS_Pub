#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Manager/GameDS_GameManager.h"

class UGameDS_GameManager;

class FGameInstanceHolder
{
protected:
	
public:
	static class UGameInstance* GameDSInstance;
	
	static void SetGameInstance(class UGameInstance* InGameInstance)
	{
		GameDSInstance = InGameInstance;
	}
	static class UGameInstance* GetGameInstance()
	{
		return GameDSInstance;
	}

	template<typename SubSystemClass>
	static SubSystemClass* GetSubSystem()
	{
		if (GameDSInstance == nullptr)
			return nullptr;
	
		return GameDSInstance->GetSubsystem<SubSystemClass>();
	}

	template<typename ManagerClass>
	static ManagerClass* GetManager()
	{
		if (GameDSInstance == nullptr)
			return nullptr;

		UGameDS_GameManager* GameManager = GameDSInstance->GetSubsystem<UGameDS_GameManager>();
		if (GameManager == nullptr)
			return nullptr;

		return GameManager->GetManagerObject<ManagerClass>();
	}

	static UWorld* GetWorld()
	{
		if (GameDSInstance == nullptr)
			return nullptr;

		return GameDSInstance->GetWorld();
	}
};
