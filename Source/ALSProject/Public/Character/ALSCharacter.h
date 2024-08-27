#pragma once

#include "CoreMinimal.h"
#include "Character/ALSBaseCharacter.h"
#include "ALSCharacter.generated.h"


UCLASS()
class ALSPROJECT_API AALSCharacter : public AALSBaseCharacter
{
	GENERATED_BODY()

public:

	/*
	* 接口函数重载
	*/
	virtual void GetTPTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel);
	virtual void GetTPPivotTarget(FTransform& ReturnValue);
	virtual void GetFPCameraTarget(FVector& ReturnValue);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual UAnimMontage* GetRollAnimation() override;

public:

	/*
	* RollMontage
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Roll")
	UAnimMontage* LandRollDefault;
	
};
