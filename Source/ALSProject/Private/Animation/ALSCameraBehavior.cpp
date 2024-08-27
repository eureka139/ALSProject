#include "Animation/ALSCameraBehavior.h"
#include "Character/ALSBaseCharacter.h"

void UALSCameraBehavior::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	UpdateCharacterInfo();
}

void UALSCameraBehavior::UpdateCharacterInfo()
{

	if (IsValid(ControlledPawn))
	{

		//Then0

		EMovementMode MovementMode;
		EALSMovementState PrevMovementState;
		EALSOverlayState OverlayState;

		AALSBaseCharacter* RefCharacter = Cast<AALSBaseCharacter>(ControlledPawn);
		IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(RefCharacter);
		ALSCharacterInterface->GetCurrentState(
			MovementMode,
			MovementState,
			PrevMovementState,
			MovementAction,
			RotationMode,
			Gait,
			Stance,
			ViewMode,
			OverlayState
		);

		//Then1
		float TPFOV;
		float FPFOV;
		IALSCameraInterface* CameraInterface = Cast<IALSCameraInterface>(ControlledPawn);
		CameraInterface->GetCameraParameters(
			TPFOV,
			FPFOV,
			bRightShoulder
		);


	}

}
