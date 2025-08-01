#include "GameDS_SoulShelterWidget.h"

void UGameDS_SoulShelterWidget::OnAppeared()
{
	Super::OnAppeared();

	OnChangeVisibility(GetVisibility());
}

void UGameDS_SoulShelterWidget::OnDisappeared()
{
	Super::OnDisappeared();

	OnChangeVisibility(GetVisibility());
}
