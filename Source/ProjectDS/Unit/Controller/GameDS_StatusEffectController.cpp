#include "GameDS_StatusEffectController.h"

#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Manager/GameDS_DataManager.h"

void UGameDS_StatusEffectController::Init(AGameDS_Character* InOwnerCharacter)
{
	if (InOwnerCharacter == nullptr)
		return;
	
	OwnerCharacter = InOwnerCharacter;
	
	for (uint8 EnumIndex = 1; EnumIndex < StaticCast<uint8>(EGameDS_StatusEffectType::Num); EnumIndex++)
		StatusEffectMap.Add(StaticCast<EGameDS_StatusEffectType>(EnumIndex));
}

void UGameDS_StatusEffectController::Tick(float DeltaTime)
{
	for (auto& PairData : StatusEffectMap)
	{
		if (PairData.Value.IsEmpty())
			continue;

		const int32 MaximumCount = PairData.Value.Num();
		int32 ActiveCount = PairData.Value.Num();
		for (FGameDS_StatusEffectInfo& StatusInfoData : PairData.Value)
		{
			StatusInfoData.RemainTime -= DeltaTime;
			
			if (StatusInfoData.RemainTime <= 0.0f)
				ActiveCount--;
		}

		if (ActiveCount == MaximumCount)
			continue;
		
		if (ActiveCount == 0)
		{
			PairData.Value.Empty();	
		}
		else
		{
			for (int32 Index = MaximumCount - 1; Index >= 0; Index--)
			{
				if (PairData.Value[Index].RemainTime <= 0.0f)
					PairData.Value.RemoveAt(Index);
			}
		}
	}
}

void UGameDS_StatusEffectController::AddStatusEffect(int32 InDataID, float InDuration/* = INDEX_NONE*/)
{
	UGameDS_DataManager* DataManager = FGameInstanceHolder::GetSubSystem<UGameDS_DataManager>();
	if (DataManager == nullptr)
		return;

	const FGameDS_StatusEffectInfoDataTable* StatusEffectRow = DataManager->GetDataInRow<FGameDS_StatusEffectInfoDataTable>(InDataID);
	if (StatusEffectRow == nullptr)
		return;

	FGameDS_StatusEffectInfo StatusEffectInfo;
	StatusEffectInfo.DataID = InDataID;
	StatusEffectInfo.RemainTime = StatusEffectRow->TotalDuration;

	if (StatusEffectInfo.RemainTime == INDEX_NONE)
		StatusEffectInfo.RemainTime = InDuration;

	if (StatusEffectMap.Find(StatusEffectRow->StatusEffectType) == nullptr)
		return;

	TArray<FGameDS_StatusEffectInfo>* StatusEffectList = StatusEffectMap.Find(StatusEffectRow->StatusEffectType);
	StatusEffectList->Add(StatusEffectInfo);
}

bool UGameDS_StatusEffectController::IsDamageable() const
{
	if (IsActiveType(EGameDS_StatusEffectType::Invincibility))
		return false;

	return true;
}

bool UGameDS_StatusEffectController::IsActiveType(EGameDS_StatusEffectType InStatusEffectType) const
{
	if (StatusEffectMap.IsEmpty())
		return false;

	if (StatusEffectMap.Find(InStatusEffectType) == nullptr)
		return false;

	return StatusEffectMap.Find(InStatusEffectType)->Num() > 0;
}