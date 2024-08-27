#pragma once

#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "ALSProject/Data/ALSStruct.h"
#include "Interface/ALSAnimationInterface.h"
#include "Animation/AnimInstance.h"
#include "ALSCharacterAnimInstance.generated.h"

class UCurveVector;
class UCurveFloat;

UCLASS()
class ALSPROJECT_API UALSCharacterAnimInstance : public UAnimInstance , public IALSAnimationInterface
{

	GENERATED_BODY()


public:

	//事件开始初始化
	virtual void NativeInitializeAnimation() override;

	//实时更新动画
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	//锚点延迟定时器
	void OnPivotDelay();

	//跳跃延迟定时器
	void OnJumpedDelay();

	//
	void PlayDynamicTransitionDelay();

	/*
	* ALSAnimationInterface
	*/

	virtual void Jumped();

	virtual void SetGroundedEntryState(EALSGroundedEntryState LocalGroundedEntryState);

	virtual void SetOverlayOverrideState(int32 LocalOverlayOverrideState);


	/*
	* AnimNotify
	*/

	UFUNCTION()
	void AnimNotify_HipsF();

	UFUNCTION()
	void AnimNotify_HipsB();

	UFUNCTION()
	void AnimNotify_HipsLF();

	UFUNCTION()
	void AnimNotify_HipsLB();

	UFUNCTION()
	void AnimNotify_HipsRF();

	UFUNCTION()
	void AnimNotify_HipsRB();

	UFUNCTION()
	void AnimNotify_Pivot();

	UFUNCTION()
	void AnimNotify_NStopL();

	UFUNCTION()
	void AnimNotify_NStopR();

	UFUNCTION()
	void AnimNotify_StopTransition();

	UFUNCTION()
	void AnimNotify_Land_Idle();

	UFUNCTION()
	void AnimNotify_CLFStop();

	UFUNCTION()
	void AnimNotify_NQuickStop();

	UFUNCTION()
	void AnimNotify_Roll_Idle();

	UFUNCTION()
	void AnimNotify_Reset_GroundedEntryState();

	/*
	* UpdateValues
	*/

	//实时更新通过接口传递过来的角色主要信息
	void UpdateCharacterInformation();

	//实时更新角色移动参数值
	void UpdateMovementValues();

	//实时更新角色旋转参数值
	void UpdateRotationValues();

	//实时更新计算角色瞄准状态下的数值并且对控制器和人物的旋转值进行处理
	void UpdateAimingValue();

	//
	void UpdateLayerValues();

	//实时更新脚步IK
	void UpdateFootIK();

	//实时更新角色在空中的数值信息
	void UpdateInAirValue();



	/*
	* Grounded
	*/

	//是否处于移动状态
	bool ShouldMoveCheck();

	//是否可以进行原地旋转
	bool CanRotateInPlace();

	//是否可以进行原地转向
	bool CanTurnInPlace();

	//是否可以进行动态过渡
	bool CanDynamicTransition();

	//检测原地转向
	void TurnInPlaceCheck();

	//原地转向（第三人称）
	void TurnInPlace(FRotator TargetRotation,float PlayRateScale,float StartTime,bool OverrideCurrent);

	//原地旋转（第一人称）
	void RotationInPlaceCheck();

	//
	void DynamicTransitionCheck();

	//
	void PlayTransition(FDynamicMontageParams Parameters);

	//
	void PlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters);


	/*
	* InAir
	*/

	float CalculateLandPrediction();


	FLeanAmount CalculateInAirLeanAmount();

	


	/*
	* FootIK
	*/

	//设置脚步锁定的数值信息（基于曲线和骨骼空间中的骨骼信息）
	void SetFootLocking(
		FName EnableFootIKCurve, 
		FName FootLockCurve, 
		FName IKFootBone, 
		float& CurrentFootLockAlpha, 
		FVector& CurrentFootLockLocation, 
		FRotator& CurrentFootLockRotation);

	//设置脚步锁定的偏移量
	void SetFootLockOffset(FVector& LocalLocation, FRotator& LocalRotation);

	//设置脚步距离地面的偏移值并将其返回
	void SetFootOffsets(
		FName EnableFootIKCurve,
		FName IKFootBone,
		FName RootBone,
		FVector& CurrentLocationTarget,
		FVector& CurrentLocationOffset,
		FRotator& CurrentRotationOffset
	);

	//设置角色盆骨处的IK偏移值
	void SetPeivisIKOffset(FVector FootOffsetLTarget, FVector FootOffsetRTarget);

	//重制角色IK的偏移值
	void ResetIKOffset();


	/*
	* Movement
	*/

	//计算角色各方向的速度混合
	FVelocityBlend CalculateVelocityBlend();

	//计算脚步对角线上的混合
	float CalculateDiagonalAmount();

	//计算角色在运动是加速度方向的相对偏移值
	FVector CalculateRelativeAccelerationAmount();

	//计算行走和奔跑之间的混合率
	float CalculateWalkRunBlend();

	//计算角色步距的混合
	float CalculateStrideBlend();

	//计算站立状态下角色的动画播放率
	float CalculateStandingPlayRate();

	//计算蹲伏状态下角色的动画播放率
	float CalculateCrouchingPlayRate();


	/*
	* Rotation
	*/

	//根据旋转值计算角色的旋转朝向枚举值
	EALSMovementDirection CalculateMovementDirection();

	//计算角色朝向的象限
	EALSMovementDirection CalculateQuadrant(
		EALSMovementDirection Current,
		float FR_Threshold,
		float FL_Threshold,
		float BR_Threshold,
		float BL_Threshold,
		float Buffer,
		float Angle
	);

	//利用阈值和控制器旋转值之间的差值来判断人物的朝向
	bool AngleInRange(
		float Angle,
		float MinAngle,
		float MaxAngle,
		float Buffer,
		bool IncreaseBuffer
	);



	/*
	* Interp
	*/

	//将速度混合进行插值过渡
	FVelocityBlend InterpVelocityBlend(FVelocityBlend Current, FVelocityBlend Target, float InterpSpeed, float DeltaTime);

	//将倾斜参数值进行插值过渡
	FLeanAmount InterpLeanAmount(FLeanAmount Current, FLeanAmount Target, float InterpSpeed, float DeltaTime);

	/*
	* Macro
	*/

	//获取骨骼动画曲线限制后的值
	float GetAnimCurveClamped(FName Name, float Bias, float ClampMin, float ClampMax);

	float GetAnimCurveCompact(FName Name);

public:


	/*
	* References
	*/

	float DeltaTimeX; //场景差量

	UPROPERTY()
	class ACharacter* ALSCharacter; //角色引用

	UPROPERTY()
	class AALSBaseCharacter* ALSBaseCharacter; //Debug角色引用

	EMovementMode MovementMode; //角色默认运动模式

	/*
	* Character Information
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector Velocity; //角色速度大小

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector Acceleration; //角色当前加速度变化量

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector MovementInput; //角色最大加速度大小

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	bool bIsMoving; //角色当前是否在移动

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	bool bHasMovingInput; //是否有运动输入

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	float Speed; //角色当前速率大小

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	float MovementInputAmount; //有运动输入则大于0无运动输入则为0（0-1）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	float AimYawRate; //控制器每一帧旋转的速度大小

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	FRotator AimingRotation; //角色当前控制器旋转值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSMovementState MovementState; //角色运动状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSMovementState PrevMovementState; //角色上一时刻运动状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSMovementAction MovementAction; //角色蒙太奇

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSRotationMode RotationMode; //角色旋转模式

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSGait Gait; //角色步态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSStance Stance; //角色姿态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSViewMode ViewMode; //角色视角模式

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSOverlayState OverlayState;  //角色叠加状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSGroundedEntryState GroundedEntryState; //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Character Information")
	int32 OverlayOverrideState;

	/*
	* Grounded
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	bool bShouldMove; //是否正在移动

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	FVelocityBlend VelocityBlend; //速度混合

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float DiagonalScaleAmount; //对角线修正值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	FVector RelativeAccelerationAmount; //角色相对加速度的朝向值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	FLeanAmount LeanAmount; //角色倾斜参数值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float WalkRunBlend; //角色行走奔跑的混合

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float StrideBlend; //角色步距的混合

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float StandPlayRate; //角色站立状态下的播放率

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float CrouchPlayRate; //角色蹲伏状态下的播放率

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	EALSMovementDirection MovementDirection = EALSMovementDirection::EMD_ForWard; //角色运动方向枚举值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float FYaw; //根据YawOffsetFB的X轴值来对应FYaw的数值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float BYaw; //根据YawOffsetFB的Y轴值来对应FYaw的数值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float LYaw; //根据YawOffsetLR的X轴值来对应FYaw的数值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float RYaw; //根据YawOffsetLR的Y轴值来对应FYaw的数值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	EALSHipsDirection TrackedHipsDirection; //根胯骨朝向枚举

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	bool bPivot; //六向状态机锚点布尔值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float RotationScale; //曲线RotationAmount所需的旋转缩放值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	bool bRotationL; //是否可以进行左向旋转

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	bool bRotationR; //是否可以进行右向旋转

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Grounded")
	float RotateRate; //原地旋转速率

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Grounded")
	TObjectPtr<UAnimSequenceBase> LeftTransitionAnimation; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Grounded")
	TObjectPtr<UAnimSequenceBase> RightTransitionAnimation; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Grounded")
	TObjectPtr<UAnimSequenceBase> LeftStopDownAnimation; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Grounded")
	TObjectPtr<UAnimSequenceBase> RightStopDownAnimation; //

	/*
	* InAir
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|InAir")
	bool bJumped; //是否处于跳跃状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|InAir")
	float JumpPlayRate; //动画跳跃播放率

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|InAir")
	float FallSpeed; //角色下落速度

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|InAir")
	float LandPrediction; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|InAir")
	TObjectPtr<UCurveFloat> LandPredictionCurve; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|InAir")
	TObjectPtr<UCurveFloat> LeanInAirCurve; //

	/*
	* TransitionAnimation
	*/



	UPROPERTY(EditDefaultsOnly, Category = "ALS|TransitionAnimation")
	TObjectPtr<UAnimSequenceBase> Land_IdleTransitionAnimation; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TransitionAnimation")
	TObjectPtr<UAnimSequenceBase> CLFStopTransitionAnimation; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TransitionAnimation")
	TObjectPtr<UAnimSequenceBase> NQuickStopTransitionAnimation; //

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TransitionAnimation")
	TObjectPtr<UAnimSequenceBase> Roll_IdleTransitionAnimation; //

	/*
	* AimingValues
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	FRotator SmoothedAimingRotation; //平滑插值控制器旋转过渡值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	FVector2D AimingAngle; //瞄准角度（控制器和Actor旋转差值）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	FVector2D SmoothedAimingAngle; //平滑瞄准过渡角度（平滑插值控制器旋转过渡值和Actor旋转差值）\

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	float AImSweepTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	FRotator SpineRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	float InputYawOffsetTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	float LeftYawTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	float RightYawTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|AimingValues")
	float ForwardYawTime;


	 
	/*
	* LayerBlending
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float EnableAimOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float BasePoseN;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float BasePoseCLF;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float LayeringSpineAdd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float LayeringHeadAdd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float LayeringArmLAdd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float LayeringArmRAdd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
		float LayeringHandL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float LayeringHandR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float EnableHandIKL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float EnableHandIKR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float ArmLLS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float ArmRLS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float ArmLMS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|LayerBlending")
	float ArmRMS;



	/*
	* FootIK
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	float FootLock_L_Alpha; //若数值为1则可以开启左脚脚步锁定

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	float FootLock_R_Alpha; //若数值为1则可以开启右脚脚步锁定

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FVector FootLock_L_Location; //骨骼空间中基于root的虚拟ik骨的左脚位置

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FRotator FootLock_L_Rotation; //骨骼空间中基于root的虚拟ik骨的左脚旋转值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FVector FootLock_R_Location; //骨骼空间中基于root的虚拟ik骨的右脚位置

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FRotator FootLock_R_Rotation; //骨骼空间中基于root的虚拟ik骨的右脚旋转值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FVector FootOffset_L_Location; //左脚脚面距离地面的Location的偏移值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FRotator FootOffset_L_Rotation; //左脚脚面位于斜面时的脚步旋转Rotation的偏移值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FVector FootOffset_R_Location; //右脚脚面距离地面的Location的偏移值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FRotator FootOffset_R_Rotation; //右脚脚面位于斜面时的脚步旋转Rotation的偏移值

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	float PelvisAlpha; //IK启用动画曲线脚整体数值(左右脚动画曲线相加除以二）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|FootIK")
	FVector PelvisOffset; //人物整体偏移值

	/*
	* TurnInPlace
	*/

	//原地转向所需的最小角度（控制器和Actor锚点差量）
	float TurnCheckMinAngle = 45.0;

	//控制器旋转所需的最小速度大小
	float AimYawRateLimit = 50.0;

	//已延迟时间
	float ElapsedDelayTime;

	//最大角度延迟时间
	float MaxAngleDelay = 0.0;

	//最小角度延迟时间
	float MinAngleDelay = 0.75;

	//执行180或者90度旋转的临界值，大于该临界值则执行180，小于则执行90
	float Turn180Threshold = 130.0;

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset N_TurnIP_L90; //站立左90转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset N_TurnIP_R90; //站立右90转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset N_TurnIP_L180; //站立左180转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset N_TurnIP_R180; //站立右180转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset CLF_TurnIP_L90; //蹲伏左90转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset CLF_TurnIP_R90; //蹲伏右90转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset CLF_TurnIP_L180; //蹲伏左180转向结构体

	UPROPERTY(EditDefaultsOnly, Category = "ALS|TurnInPlace")
	FTurnInPlaceAsset CLF_TurnIP_R180; //蹲伏右180转向结构体

	/*
	* RotateInPlace
	*/


	//原地旋转最小阈值
	float RotateMinThreshold = -50.0;

	//原地旋转最大阈值
	float RotateMaxThreshold = 50.0;
	
	//瞄准Yaw值最小范围值
	float AimYawRateMinRange = 90.0;

	//瞄准Yaw值最大范围值
	float AimYawRateMaxRange = 270.0;

	//原地旋转最小播放率
	float MinPlayRate = 1.15;

	//原地旋转最大播放率
	float MaxPlayRate = 3.0;


	/*
	* BlendCurves
	*/

	UPROPERTY(EditDefaultsOnly, Category = "ALS|BlendCurves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve; //对角线修改曲线值

	UPROPERTY(EditDefaultsOnly, Category = "ALS|BlendCurves")
	TObjectPtr<UCurveFloat> StrideBlendNWalk; //站立状态下行走步距的混合

	UPROPERTY(EditDefaultsOnly, Category = "ALS|BlendCurves")
	TObjectPtr<UCurveFloat> StrideBlendNRun; //站立状态下奔跑步距的混合

	UPROPERTY(EditDefaultsOnly, Category = "ALS|BlendCurves")
	TObjectPtr<UCurveFloat> StrideBlendCWalk; //蹲伏状态下行走步距的混合

	UPROPERTY(EditDefaultsOnly, Category = "ALS|BlendCurves")
	TObjectPtr<UCurveVector> YawOffsetFB; //角色移动时Yaw的前后偏移值

	UPROPERTY(EditDefaultsOnly, Category = "ALS|BlendCurves")
	TObjectPtr<UCurveVector> YawOffsetLR; //角色移动时Yaw的左右偏移值

	/*
	* Configure
	*/
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float VelocityBlendInterpSpeed = 12.0; //速度混合的插值速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float GroundedLeanInterpSpeed = 4.0; //人物倾斜的插值速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float AnimatedWalkSpeed = 150.0; //人物动画的行走速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float AnimatedRunSpeed = 350.0; //人物动画的奔跑速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float AnimatedSprintSpeed = 600.0; //人物动画的冲刺速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float AnimatedCrouchSpeed = 150.0; //人物动画的蹲伏速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float TriggerPivotSpeedLimit = 200.0; //人物冲刺偏移效果速度最小值判断

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float SmoothedAimingRotationInterpSpeed = 10.0; //人物平滑控制器旋转值插值速度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float IKTraceDistanceAboveFoot = 50.0; //IK检测从ik_foot_l/r向正方向延长的长度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float IKTraceDistanceBelowFoot = 45.0; //IK检测从ik_foot_l/r向负方向延长的长度 

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	float FootHeight = 13.5; //IK骨骼ik_foot_l/r距离地面的高度

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Configure")
	bool bCanPlayDynamicTransition = true; //

	float InAirLeanInterpSpeed = 4.0;

	float InputYawOffsetInterpSpeed = 8.0;



	/*
	* Timer
	*/

	FTimerHandle OnPivotTimer; //六向状态机锚点布尔更改时间轴句柄

	FTimerHandle OnJumpedTimer; //跳跃布尔值更改时间轴句柄

	FTimerHandle PlayDynamicTransitionTimer; //
};
