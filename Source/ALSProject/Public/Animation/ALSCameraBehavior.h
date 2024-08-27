#pragma once

#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "Interface/ALSCharacterInterface.h"
#include "Interface/ALSCameraInterface.h"
#include "Animation/AnimInstance.h"
#include "ALSCameraBehavior.generated.h"

UCLASS()
class ALSPROJECT_API UALSCameraBehavior : public UAnimInstance
{
	GENERATED_BODY()

public:

	//实时更新动画
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	void UpdateCharacterInfo();

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class APawn* ControlledPawn; //受控的Pawn

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class APlayerController* PlayerController; //玩家控制器

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	EALSGait Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	EALSMovementState MovementState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	EALSMovementAction MovementAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	EALSRotationMode RotationMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	EALSStance Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	EALSViewMode ViewMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	bool bRightShoulder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|CharacterInfo")
	bool bDebugView;
	
};
