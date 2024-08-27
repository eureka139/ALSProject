

#pragma once

#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "Interface/ALSCharacterInterface.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ALSAnimNotifyStateEarlyBlendOut.generated.h"

/**
 * 
 */
UCLASS()
class ALSPROJECT_API UALSAnimNotifyStateEarlyBlendOut : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;


public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	TObjectPtr<UAnimMontage> ThisMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	float BlendOutTime = 0.25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckMovementState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EALSMovementState MovementStateEquals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckStance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EALSStance StanceEquals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckMovementInput;



};
