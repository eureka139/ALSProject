#pragma once

#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "UObject/Interface.h"
#include "ALSCharacterInterface.generated.h"

UINTERFACE(MinimalAPI)
class UALSCharacterInterface : public UInterface
{

	GENERATED_BODY()

};

class ALSPROJECT_API IALSCharacterInterface
{

	GENERATED_BODY()

public:

	/*
	* Character Information
	* 此处两个接口函数是将人物中的状态信息同步到动画实例当中
	*/

	//获取角色当前状态
	virtual void GetCurrentState(
		EMovementMode & IPawnMovementMode,
		EALSMovementState & IMovementState,
		EALSMovementState & IPrevMovementState,
		EALSMovementAction & IMovementAction,
		EALSRotationMode & IRotationMode,
		EALSGait & IActualGait,
		EALSStance & IActualStance,
		EALSViewMode & IViewMode,
		EALSOverlayState & IOverlayState
	);

	//获取角色主要信息
	virtual void GetEssentialValues(
		FVector & IVelocity,
		FVector & IAcceleration,
		FVector & IMovementInput,
		bool & IbIsMoving,
		bool & IbHasMovingInput,
		float & ISpeed,
		float & IMovementInputAmount,
		float & IAimYawRate,
		FRotator & IAimingRotation
	);

	/*
	* Character State
	*/

	
	virtual void SetMovementState(EALSMovementState NewMovementState, bool bForce = false);
	
	virtual void SetMovementAction(EALSMovementAction NewMovementAction, bool bForce = false);
	
	virtual void SetRotationMode(EALSRotationMode NewRotationMode, bool bForce = false);
	
	virtual void SetGait(EALSGait NewGait, bool bForce = false);
	
	virtual void SetViewMode(EALSViewMode NewViewMode, bool bForce = false);
	
	virtual void SetOverlayState(EALSOverlayState NewOverlayState, bool bForce = false);

	virtual void SetStance(EALSStance NewStance, bool bForce = false);

};
