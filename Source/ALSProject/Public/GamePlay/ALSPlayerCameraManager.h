#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "ALSPlayerCameraManager.generated.h"

UCLASS()
class ALSPROJECT_API AALSPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	AALSPlayerCameraManager();

	//Pawn被控制时调用此函数
	void OnPossess(APawn* NewPawn);
	//自定义摄像机的各个参数
	UFUNCTION(BlueprintCallable)
	void CustomCameraBehavior(FVector& Location, FRotator& Rotation, float& FOV);
	//从摄像机骨骼中获取曲线信息
	float GetCameraBehaviorParam(FName CurvesName);
	//计算各个轴值的滞后速度(主要体现在人物移动时的滞后速度)
	FVector CalculateAxisLndependentLag(FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeed);

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* CameraBehavior; //摄像机骨骼

	UPROPERTY()
	class APawn* ControlledPawn; //受控的Pawn

	FRotator TargetCameraRotation; //正常视角摄像机旋转值

	FRotator DebugViewRotation = FRotator(-5.0f, 100.0f, 0.0f); //Debug视角的摄像机旋转值

	FTransform SmoothTargetPivot; //在移动时滞后速度的锚点的位置信息

	FVector PivotLocation; //锚点信息的进一步修改值，此向量是用来逼近摄像机位置

	FVector DebugViewOffset = FVector(350, 0, 50); //Debug模式下的视口偏移量

	FVector TargetCameraLocation; //摄像机最终的参数值
};
