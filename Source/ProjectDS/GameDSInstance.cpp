#include "GameDSInstance.h"

#include "GameInstanceHolder.h"

UGameDSInstance::UGameDSInstance()
{
}

void UGameDSInstance::Init()
{
	FGameInstanceHolder::SetGameInstance(this);

	Super::Init();
}

void UGameDSInstance::Shutdown()
{
	Super::Shutdown();

	FGameInstanceHolder::SetGameInstance(nullptr);
}
