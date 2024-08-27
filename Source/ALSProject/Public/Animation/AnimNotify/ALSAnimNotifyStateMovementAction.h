

#pragma once

#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "Interface/ALSCharacterInterface.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ALSAnimNotifyStateMovementAction.generated.h"

/**
 * 
 */
UCLASS()
class ALSPROJECT_API UALSAnimNotifyStateMovementAction : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	
	//通知开始时调用此函数（于动画序列中添加通知）
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	//通知结束时调用此函数（于动画序列中添加通知）
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	//获取通知的命名
	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EALSMovementAction MovementAction; //动画序列中调用通知时可更改此枚举值（Rolling）

};
