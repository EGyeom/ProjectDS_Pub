													#pragma once
#include "ProjectDS/Unit/GameDS_UnitAnimInstance.h"
#include "GameDS_EnemyAnimInstance.generated.h"

UCLASS()
class UGameDS_EnemyAnimInstance : public UGameDS_UnitAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void InitAnimationAlias() override;
private:

};
