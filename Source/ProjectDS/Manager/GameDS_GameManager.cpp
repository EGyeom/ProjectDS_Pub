#include "GameDS_GameManager.h"

#include "GameDS_SummonManager.h"
#include "GameDS_UnitManager.h"
#include "Debug/GameDS_DebugManager.h"
#include "Object/GameDS_ManagerObject.h"
#include "ProjectDS/GameInstanceHolder.h"
#include "ProjectDS/Skill/GameDS_SkillDefine.h"
#include "ProjectDS/Unit/GameDS_Character.h"
#include "ProjectDS/Unit/Enemy/GameDS_Boss.h"

void UGameDS_GameManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ManagingObjectList.Add(NewObject<UGameDS_DebugManager>());
	ManagingObjectList.Add(NewObject<UGameDS_SummonManager>());

	// Command
	FConsoleCommandWithArgsDelegate ExecuteConsoleCommandDelegate;
	ExecuteConsoleCommandDelegate.BindUObject(this, &UGameDS_GameManager::ExecuteConsoleCommand);
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("GameDebug"),TEXT("디버그 콘솔 치트"), ExecuteConsoleCommandDelegate);

	bIsCreateOnRunning = GIsRunning;
}

void UGameDS_GameManager::ExecuteConsoleCommand(const TArray<FString>& Arg)
{
	if (Arg.IsEmpty())
		return;

	if (Arg[0].Compare(TEXT("ChangeDebugMode")) == 0)
	{
		if (UGameDS_DebugManager* DebugManager = GetManagerObject<UGameDS_DebugManager>())
			DebugManager->SetDebugMode(GetGameInstance());
	}
	else if (Arg[0].Compare(TEXT("ChangeDebugDistance")) == 0)
	{
		if (Arg.Num() == 1)
			return;
		
		const float InDistanceValue = FCString::Atof(*Arg[1]);
		if (UGameDS_DebugManager* DebugManager = GetManagerObject<UGameDS_DebugManager>())
			DebugManager->SetDebugDistance(InDistanceValue);
	}
	else if (Arg[0].Compare(TEXT("ChangeCollisionDuration")) == 0)
	{
		if (Arg.Num() == 1)
			return;

		const float InDuration = FCString::Atof(*Arg[1]);
		if (UGameDS_DebugManager* DebugManager = GetManagerObject<UGameDS_DebugManager>())
			DebugManager->SetCollisionDuration(InDuration);
	}
	else if (Arg[0].Compare(TEXT("SetDamage")) == 0)
	{
		if (Arg.Num() == 1)
			return;

		const float InValue = FCString::Atof(*Arg[1]);
		
		UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>();
		if (UnitManager == nullptr)
			return;
		
		AGameDS_Character* LocalCharacter = UnitManager->GetLocalCharacter();
		if (LocalCharacter == nullptr)
			return;

		if (LocalCharacter->OnDamageDelegate.IsBound())
		{
			FGameDS_DamageInfo DamageInfo;
			DamageInfo.AttackedUnitID = LocalCharacter->GetUnitID();
			DamageInfo.DamageValue = InValue;
			DamageInfo.DamageType = EGameDS_DamageType::Light;
			DamageInfo.HitLocation = LocalCharacter->GetActorLocation();
			
			LocalCharacter->OnDamageDelegate.Broadcast(DamageInfo);
		}
	}
	else if (Arg[0].Compare(TEXT("ChangePhase")) == 0)
	{
		if (Arg.Num() == 1)
			return;
		
		const int InValue = FCString::Atof(*Arg[1]);

		UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>();
		if (UnitManager == nullptr)
			return;
		
		AGameDS_Boss* BossCharacter = Cast<AGameDS_Boss>(UnitManager->GetCharacter(UnitManager->GetBossID()));
		
		if (BossCharacter == nullptr)
			return;

		BossCharacter->ChangePhase(InValue);
	}
	else if (Arg[0].Compare(TEXT("SetBossDamage")) == 0)
	{
		if (Arg.Num() == 1)
			return;

		const float InValue = FCString::Atof(*Arg[1]);

		UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>();
		if (UnitManager == nullptr)
			return;

		AGameDS_Character* BossCharacter = UnitManager->GetCharacter(UnitManager->GetBossID());
		if (BossCharacter == nullptr)
			return;

		if (BossCharacter->OnDamageDelegate.IsBound())
		{
			FGameDS_DamageInfo DamageInfo;
			DamageInfo.AttackedUnitID = UnitManager->GetBossID();
			DamageInfo.DamageValue = InValue;
			DamageInfo.DamageType = EGameDS_DamageType::Light;
			DamageInfo.HitLocation = BossCharacter->GetActorLocation();

			BossCharacter->OnDamageDelegate.Broadcast(DamageInfo);
		}
	}
}

void UGameDS_GameManager::Tick(float DeltaTime)
{
	for (UGameDS_ManagerObject* ManagingObject : ManagingObjectList)
	{
		if (ManagingObject == nullptr)
			continue;
		
		ManagingObject->Tick(DeltaTime, GetGameInstance());
	}

	if (bRequestRespawn)
	{
		if (RespawnRemainTime > 0.0f)
		{
			RespawnRemainTime -= DeltaTime;
		}
		else
		{
			RespawnRemainTime = 0.0f;
			bRequestRespawn = false;

			if (UGameDS_UnitManager* UnitManager = FGameInstanceHolder::GetSubSystem<UGameDS_UnitManager>())
			{
				UnitManager->RespawnAllUnit();
			}
		}
	}
}

bool UGameDS_GameManager::IsTickable() const
{
	return bIsCreateOnRunning;
}

TStatId UGameDS_GameManager::GetStatId() const
{
	return GetStatID();
}

void UGameDS_GameManager::RequestRespawnAllUnit(float InDelay)
{
	bRequestRespawn = true;
	RespawnRemainTime = InDelay;
}
