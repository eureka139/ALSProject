#pragma once

#include "ALSEnum.h"
#include "Engine/Datatable.h"
#include "ALSStruct.generated.h"


class UCurveVector;
class UCurveFloat;
class UAnimMontage;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FALSComponentAndTransform
{
	GENERATED_BODY();

	UPrimitiveComponent* MantleComponent;

	FTransform MantleTransform;

};

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_BODY();

};

USTRUCT(BlueprintType)
struct FCameraSettingsGait
{
	GENERATED_BODY();

};

USTRUCT(BlueprintType)
struct FCameraSettingsState
{
	GENERATED_BODY();

};

/*
* 动态蒙太奇动画参数值
*/

USTRUCT(BlueprintType)
struct FDynamicMontageParams
{
	GENERATED_BODY();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	TObjectPtr<UAnimSequenceBase> Animation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	float BlendInTime;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	float BlendOutTime;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	float PlayRate;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	float StartTime;
};

/*
* 人物进行移动时倾斜的偏移量
*/

USTRUCT(BlueprintType)
struct FLeanAmount
{
	GENERATED_BODY();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	float LR;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Grounded")
	float FB;

};

USTRUCT(BlueprintType)
struct FMantleAsset
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	TObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	TObjectPtr<UCurveVector> PositionCorrectionCurves;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	FVector StartingOffset;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float LowHeight;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float LowPlayRate;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float LowStartPosition;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float HighHeight;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float HighPlayRate;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float HighStartPosition;

};

USTRUCT(BlueprintType)
struct FMantleParams
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	TObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	TObjectPtr<UCurveVector> PositionCorrectionCurves;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float StratingPosition;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float PlayRate;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	FVector StartingOffset;

};

USTRUCT(BlueprintType)
struct FMantleTraceSettings
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float MaxLedgeHeight;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float MinLedgeHeight;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float ReachDistance;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float ForwardTraceRadius;

	UPROPERTY(EditAnywhere, Category = "MantleSystem")
	float DownwardTraceRadius;

};

/*
* 人物移动的基础信息设置结构体
*/

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float RunSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SprintSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurves = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurves = nullptr;

};

/*
* 基于人物基础设置结构体嵌套的人物步态结构体
*/

USTRUCT(BlueprintType)
struct FMovementSettingsStance
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	struct FMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	struct FMovementSettings Crouching;

};

/*
* 基于人物步态结构体嵌套的人物状态结构体
*/

USTRUCT(BlueprintType)
struct FMovementSettingsState : public FTableRowBase
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	struct FMovementSettingsStance VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	struct FMovementSettingsStance LookingDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	struct FMovementSettingsStance Aiming;

};

USTRUCT(BlueprintType)
struct FRotationInPlaceAsset
{
	GENERATED_BODY();

};

USTRUCT(BlueprintType)
struct FTurnInPlaceAsset
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|TurnInPlace")
	TObjectPtr<UAnimSequenceBase> Animation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|TurnInPlace")
	float AnimatedAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|TurnInPlace")
	FName SlotName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|TurnInPlace")
	float PlayRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|TurnInPlace")
	bool ScaleTurnAngle;


};

/*
* 人物速度混合，主要用于六向状态机中，辨别人物移动方向
*/

USTRUCT(BlueprintType)
struct FVelocityBlend
{
	GENERATED_BODY();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float F; //前向

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float B; //后向

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float L; //左向

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float R; //右向

};

USTRUCT(BlueprintType)
struct FAnimCurveCreationDate
{

	GENERATED_BODY();

	int32 FrameNumber;

	float CurvesValue;

};

USTRUCT(BlueprintType)
struct FAnimCurveCreationParams
{

	GENERATED_BODY();

	FName CurveName;

	bool KeyEachFrame;

	TArray<FAnimCurveCreationDate> Key;


};