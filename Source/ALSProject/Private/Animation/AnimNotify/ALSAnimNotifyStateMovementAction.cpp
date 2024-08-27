#include "Animation/AnimNotify/ALSAnimNotifyStateMovementAction.h"
#include "Character/ALSBaseCharacter.h"

void UALSAnimNotifyStateMovementAction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	/*
	* 通过接口函数来调用人物中重载的接口事件
	* 通知开始时将人物的MovementAction更改为Rolling
	*/

	IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(MeshComp->GetOwner());
	if (ALSCharacterInterface)
	{
		ALSCharacterInterface->SetMovementAction(MovementAction);
	}
	
}

void UALSAnimNotifyStateMovementAction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	/*
	* 通过接口函数来调用人物中重载的接口事件
	* 通知结束时判断人物当前的MovementAction是否为Rolling
	* 若为Rolling将人物的MovementAction更改为None
	*/

	AALSBaseCharacter* BaseCharacter = Cast<AALSBaseCharacter>(MeshComp->GetOwner());
	if (BaseCharacter && BaseCharacter->MovementAction == MovementAction)
	{
		IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(MeshComp->GetOwner());
		if (ALSCharacterInterface)
		{
			ALSCharacterInterface->SetMovementAction(EALSMovementAction::EMA_None);
		}
	}
}

FString UALSAnimNotifyStateMovementAction::GetNotifyName_Implementation() const
{

	/*
	* 更改动画通知的命名
	*/

	FString Name(TEXT("Movement Action: "));
	Name.Append(StaticEnum<EALSMovementAction>()->GetNameStringByValue(static_cast<int64>(MovementAction)));
	return Name; 
}
