#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ALSPlayerController.generated.h"

UCLASS()
class ALSPROJECT_API AALSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	//Pawn受控时调用此函数
	virtual void OnPossess(APawn* InPawn);
	
};
