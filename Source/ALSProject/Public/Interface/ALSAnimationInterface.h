#pragma once

#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "UObject/Interface.h"
#include "ALSAnimationInterface.generated.h"

UINTERFACE(MinimalAPI)
class UALSAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSPROJECT_API IALSAnimationInterface
{
	GENERATED_BODY()

public:

	virtual void Jumped();

	virtual void SetGroundedEntryState(EALSGroundedEntryState LocalGroundedEntryState);

	virtual void SetOverlayOverrideState(int32 LocalOverlayOverrideState);
	
};
