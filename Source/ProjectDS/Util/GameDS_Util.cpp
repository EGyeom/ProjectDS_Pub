#include "GameDS_Util.h"

void FGameDS_Util::ParsingUnitInfo(const FGameDS_HeroCreateInfo& InHeroCreateInfo, FGameDS_HeroStatInfo& HeroStatInfo, FGameDS_UnitStatInfo& UnitStatInfo)
{
	const TArray<FString>& HeroStatList = InHeroCreateInfo.HeroStatList;

	if (HeroStatList.IsValidIndex(0))
		HeroStatInfo.Level = FCString::Atoi(*HeroStatList[0]);

	if (HeroStatList.IsValidIndex(1))
		HeroStatInfo.VitalityPoint = FCString::Atoi(*HeroStatList[1]);
		
	if (HeroStatList.IsValidIndex(2))
		HeroStatInfo.EndurancePoint = FCString::Atoi(*HeroStatList[2]);
	
	if (HeroStatList.IsValidIndex(3))
		HeroStatInfo.StrengthPoint = FCString::Atoi(*HeroStatList[3]);

	if (HeroStatList.IsValidIndex(4))
		HeroStatInfo.IntelligencePoint = FCString::Atoi(*HeroStatList[4]);

	const TArray<FString>& UnitStatList = InHeroCreateInfo.UnitStatList;

	if (UnitStatList.IsValidIndex(0))
		UnitStatInfo.Health = FCString::Atoi(*UnitStatList[0]);

	if (UnitStatList.IsValidIndex(1))
		UnitStatInfo.Focus = FCString::Atoi(*UnitStatList[1]);

	if (UnitStatList.IsValidIndex(2))
		UnitStatInfo.Stamina = FCString::Atoi(*UnitStatList[2]);

	if (UnitStatList.IsValidIndex(3))
		UnitStatInfo.PhysicalDefence = FCString::Atoi(*UnitStatList[3]);
	
	if (UnitStatList.IsValidIndex(4))
		UnitStatInfo.MagicDefence = FCString::Atoi(*UnitStatList[4]);
}
