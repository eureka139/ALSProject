#include "Animation/AnimNotify/ALSAnimNotifyStateEarlyBlendOut.h"
#include "Character/ALSBaseCharacter.h"

void UALSAnimNotifyStateEarlyBlendOut::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{

	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	TObjectPtr<UAnimInstance> AnimInstance;
	TObjectPtr<AActor> OwningActor;

	if (MeshComp->GetOwner() &&
		MeshComp->GetAnimInstance())
	{
		AnimInstance = MeshComp->GetAnimInstance();
		OwningActor = MeshComp->GetOwner();
		AALSBaseCharacter* BaseCharacter = Cast<AALSBaseCharacter>(OwningActor);
		if (BaseCharacter)
		{
			//Then0

			if (bCheckMovementState)
			{
				if (BaseCharacter->MovementState == MovementStateEquals)
				{
					AnimInstance->Montage_Stop(
						BlendOutTime,
						ThisMontage
					);
				}
			}


			//Then1

			if (bCheckStance)
			{
				if (BaseCharacter->Stance == StanceEquals)
				{
					AnimInstance->Montage_Stop(
						BlendOutTime,
						ThisMontage
					);
				}
			}


			//Then2

			if (bCheckMovementInput)
			{
				if (BaseCharacter->bHasMovementInput)
				{
					AnimInstance->Montage_Stop(
						BlendOutTime,
						ThisMontage
					);
				}
			}
		}
	}
}

FString UALSAnimNotifyStateEarlyBlendOut::GetNotifyName_Implementation() const
{
	return FString("EarlyBlendOut");
}
