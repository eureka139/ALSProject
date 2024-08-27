#include "Interface/ALSCharacterInterface.h"


void IALSCharacterInterface::GetCurrentState(EMovementMode& IPawnMovementMode, EALSMovementState& IMovementState, EALSMovementState& IPrevMovementState, EALSMovementAction& IMovementAction, EALSRotationMode& IRotationMode, EALSGait& IActualGait, EALSStance& IActualStance, EALSViewMode& IViewMode, EALSOverlayState& IOverlayState)
{

}

void IALSCharacterInterface::GetEssentialValues(FVector& Velocity, FVector& Acceleration, FVector& MovementInput, bool& bIsMoving, bool& bHasMovingInput, float& Speed, float& MovementInputAmount, float& AimYawRate, FRotator& AimingRotation)
{

}

void IALSCharacterInterface::SetMovementState(EALSMovementState NewMovementState, bool bForce)
{
}

void IALSCharacterInterface::SetMovementAction(EALSMovementAction NewMovementAction, bool bForce)
{
}

void IALSCharacterInterface::SetRotationMode(EALSRotationMode NewRotationMode, bool bForce)
{
}

void IALSCharacterInterface::SetGait(EALSGait NewGait, bool bForce)
{
}

void IALSCharacterInterface::SetViewMode(EALSViewMode NewViewMode, bool bForce)
{
}

void IALSCharacterInterface::SetOverlayState(EALSOverlayState NewOverlayState, bool bForce)
{
}

void IALSCharacterInterface::SetStance(EALSStance NewStance, bool bForce)
{
}
