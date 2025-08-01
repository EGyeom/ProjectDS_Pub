#include "GameDS_SummonManager.h"

#include "ProjectDS/Unit/Summon/GameDS_Summon.h"

void UGameDS_SummonManager::Tick(float DeltaTime, UGameInstance* InGameInstance)
{
	Super::Tick(DeltaTime, InGameInstance);

	RemoveExpiredSummon(InGameInstance);
}

void UGameDS_SummonManager::AddSummon(AGameDS_Summon* InSummon)
{
	if (InSummon == nullptr)
		return;

	InSummon->SummonDespawnDelegate.BindUObject(this, &UGameDS_SummonManager::RemoveSummonCallBack);
	SummonList.Add(InSummon);
	
	if (SummonListBySkillID.Find(InSummon->GetSkillID()))
	{
		SummonListBySkillID.Find(InSummon->GetSkillID())->SummonArray.Add(InSummon);
	}
	else
	{
		SummonListBySkillID.Add(InSummon->GetSkillID(), FGameDS_SummonArrayStruct(InSummon));
	}
}

void UGameDS_SummonManager::RemoveExpiredSummon(UGameInstance* InGameInstance)
{
	if (InGameInstance == nullptr)
		return;

	for (int32 Index = ExpiredSummonList.Num() - 1; Index >= 0; Index--)
	{
		InGameInstance->GetWorld()->DestroyActor(ExpiredSummonList[Index]);
	}

	ExpiredSummonList.Empty();
}

void UGameDS_SummonManager::RemoveSummonCallBack(AGameDS_Summon* InSummon)
{
	for (auto Summon : SummonList)
	{
		if (Summon->GetName().Equals(InSummon->GetName()))
		{
			ExpiredSummonList.Add(Summon);
			SummonList.Remove(Summon);
			if (SummonListBySkillID.Find(Summon->GetSkillID()))
			{
				FGameDS_SummonArrayStruct* SummonArrayStruct = SummonListBySkillID.Find(Summon->GetSkillID());

				SummonArrayStruct->SummonArray.Remove(Summon);
			}
			break;
		}
	}
}

AGameDS_Summon* UGameDS_SummonManager::MultiSpawnSummonNearbyCharacter(AGameDS_Character* Target, int32 SkillID)
{
	if (Target == nullptr)
		return nullptr;

	float MinDistance = -1.f;

	FGameDS_SummonArrayStruct* SummonArrayStruct = SummonListBySkillID.Find(SkillID);
	if (SummonArrayStruct == nullptr)
		return nullptr;
	
	if (SummonArrayStruct->SummonArray.IsEmpty())
		return nullptr;

	AGameDS_Summon* NearestSummon = nullptr;

	for (auto Summon : SummonArrayStruct->SummonArray)
	{
		float CurrnetDistance = FVector::Dist2D(Summon->GetActorLocation(), Target->GetActorLocation());
		if (MinDistance < 0 || CurrnetDistance < MinDistance)
		{
			MinDistance = CurrnetDistance;
			NearestSummon = Summon;
		}
	}

	return NearestSummon;
}