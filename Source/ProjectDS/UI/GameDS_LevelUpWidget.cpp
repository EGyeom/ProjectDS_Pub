#include "GameDS_LevelUpWidget.h"

#include "ProjectDS/Manager/GameDS_UIManager.h"

void UGameDS_LevelUpWidget::OnAppeared()
{
	Super::OnAppeared();
}

void UGameDS_LevelUpWidget::OnDisappeared()
{
	Super::OnDisappeared();

	if (UGameDS_UIManager* const UIManager = UGameInstance::GetSubsystem<UGameDS_UIManager>(GetGameInstance()))
		UIManager->SetInputMode(EGameDS_InputMode::GameOnly);
}
