#pragma once

#include "InputActionValue.h"
#include "CoreMinimal.h"
#include "ALSProject/Data/ALSEnum.h"
#include "ALSProject/Data/ALSStruct.h"
#include "Interface/ALSCameraInterface.h"
#include "Interface/ALSCharacterInterface.h"
#include "Interface/ALSAnimationInterface.h"
#include "GameFramework/Character.h"
#include "ALSBaseCharacter.generated.h"

UCLASS()
class ALSPROJECT_API AALSBaseCharacter : public ACharacter , public IALSCameraInterface , public IALSCharacterInterface
{
	GENERATED_BODY()

public:



	/*
	* CameraSystem
	* 
	* 重载接口函数
	*/
	
	virtual void GetTPTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel);
	virtual void GetTPPivotTarget(FTransform& ReturnValue);
	virtual void GetFPCameraTarget(FVector& ReturnValue);
	virtual void GetCameraParameters(float& TPFOV, float& FPFOV, bool& RightShoulder);

	/*
	* CharacterState
	* 
	* 通过接口函数来进行角色各枚举的更替
	*/

	//当运动模式更改时，通过接口的方式，修改人物的状态信息
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	//开始蹲伏时调用此函数
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	//结束蹲伏时调用此函数
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	//角色落地时调用此函数
	virtual void Landed(const FHitResult& Hit) override;


	virtual void SetMovementState(EALSMovementState NewMovementState,bool bForce = false);
	virtual void SetMovementAction(EALSMovementAction NewMovementAction, bool bForce = false);
	virtual void SetRotationMode(EALSRotationMode NewRotationMode, bool bForce = false);
	virtual void SetGait(EALSGait NewGait, bool bForce = false);
	virtual void SetViewMode(EALSViewMode NewViewMode, bool bForce = false);
	virtual void SetOverlayState(EALSOverlayState NewOverlayState, bool bForce = false);
	virtual void SetStance(EALSStance NewStance, bool bForce = false);

	/*
	* 将Characte中计算的数值信息传入接口函数中,从而达到与动画实例进行通讯的功能
	*/
	//获取角色当前状态
	virtual void GetCurrentState(
		EMovementMode& IPawnMovementMode,
		EALSMovementState& IMovementState,
		EALSMovementState& IPrevMovementState,
		EALSMovementAction& IMovementAction,
		EALSRotationMode& IRotationMode,
		EALSGait& IActualGait,
		EALSStance& IActualStance,
		EALSViewMode& IViewMode,
		EALSOverlayState& IOverlayState
	);
	//获取角色主要信息
	virtual void GetEssentialValues(
		FVector& IVelocity,
		FVector& IAcceleration,
		FVector& IMovementInput,
		bool& IbIsMoving,
		bool& IbHasMovingInput,
		float& ISpeed,
		float& IMovementInputAmount,
		float& IAimYawRate,
		FRotator& IAimingRotation
	);
	/*
	* Normal
	*/

	//构造函数
	AALSBaseCharacter();

	//初始化函数
	virtual void BeginPlay() override;
	
	//实时更新函数
	virtual void Tick(float DeltaTime) override;

	//跳跃时执行此函数
	UFUNCTION(BlueprintCallable)
	void BlueprintOnJumped(); //因为cpp中无OnJumped继承函数，所以将其封装成函数在编辑器中执行

	/*
	* EssentialInformation
	*/

	//设置角色需要的基本数值
	void SetEssentialValue();

	//计算每一帧加速度变化量大小（Vt - V0)/ DeltaTime
	FVector CalculateAcceleration();
	
	//利用计算机计算原理，来计算上一帧的数值大小
	void CacheValues();

	/*
	* StateChanges
	*/

	//游戏开始运行时
	void OnBeginPlay();

	//从数据表中获取对应的信息，然后将其赋值给结构体
	void SetMovementModel();

	/*
	* MovementSystem
	*/

	//实时更新角色运动参数值
	void UpdateCharacterMovement();

	//获取允许的步态
	EALSGait GetAllowedGait();

	//是否可以冲刺
	bool CanSprint();

	//获取实际的步态
	EALSGait GetActualGait(EALSGait AllowedGait);

	//动态更新角色运动信息
	void UpdateDynamicMovementSettings(EALSGait AllowedGait);

	//将数据表中的数据赋给CurrentMovementSettings
	FMovementSettings GetTargetMovementSettings();
	 
	//获取速度的映射（0-3）
	float GetMappedSpeed();

	/*
	* RotationSystem
	*/

	//更新人物位于地面状态下的旋转
	void UpdateGroundedRotation();

	//是否能在运动是更新角色旋转的数值
	bool CanUpdateMovingRotation();

	//实时平滑更新人物的朝向
	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed);

	//计算地面状态下角色旋转时的速率（插值速度）
	float CalculateGroundedRotationRate();

	//限制角色的旋转值，使其不超过界限（第一人称或者瞄准状态下才能调用）
	void LimitRotation(float AimYawMin,float AimYawMax,float InterpSpeed);

	//实时更新在空中时的角色旋转朝向
	void UpdateInAirRotation();

	/*
	* MantleSystem
	*/

	//检测是否能够进行攀爬操作，并为攀爬提供所需的参数及条件
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	bool MantleCheck(FMantleTraceSettings TraceSettings,ETraceTypeQuery DebugType);

	//获取胶囊体基础位置Z轴(0 - 90)
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	FVector GetCalpsuleBaseLocation(float ZOffset);

	//根据检测墙体上方的位置来获取胶囊体位置墙体上方的锚点位置
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	FVector GetCapsuleLocationFormBase(FVector BaseLocation, float ZOffset);

	//判断墙体上方是否能够容纳一整个胶囊体
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	bool CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TraceLocation, float HeightOffset, float RadiusOffset);

	//开始进行攀爬操作
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	void MantleStart(FALSComponentAndTransform MantleLedgeWS);

	//从编辑器中重载此函数，将各个状态对应的结构体信息赋值
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|MantleSystem")
	FMantleAsset GetMantleAsset(EALSMantleType LocalMantleType);

	//时间轴更新时调用此函数
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	void MantleUpdate(float BlendIn);

	//时间轴结束时调用此函数
	UFUNCTION(BlueprintCallable, Category = "ALS|MantleSystem")
	void MantleEnd();

	/*
	* Roll
	*/

	//落地时翻滚事件
	void BreakfallEvent();

	//翻滚事件
	void RollEvent();

	//于子类中重载翻滚动画
	virtual UAnimMontage* GetRollAnimation();


	/*
	* InAir
	*/

	//落地时调用此定时器函数，重置人物的制动摩擦力因子为0
	void OnLandFrictionReset();

	//通过定时器函数将布尔值重置为false
	void BreakfallReset();



	/*
	* RagdollSystem
	*/



	void RagdollStart();



	/*
	* Input
	*/

	//设置玩家输入组件
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//获取玩家移动输入参数值，并判断是否是前向输入
	void PlayerMovementInput(bool bIsForwardAxis ,float Value);

	//向前/后移动操作
	void ForwardMovementAction(const FInputActionValue& Value);

	//向右/左移动操作
	void RightMovementAction(const FInputActionValue& Value);

	//向上/下移动视口
	void LookUpAction(const FInputActionValue& Value);

	//向右/左移动视口
	void LookRightAction(const FInputActionValue& Value);

	//跳跃
	void JumpAction(const FInputActionValue& Value);

	//行走
	void WalkAction(const FInputActionValue& Value);

	//冲刺
	void SprintAction(const FInputActionValue& Value);

	//蹲伏
	void CrouchAction(const FInputActionValue& Value);

	/*
	* Tool
	*/

	//获取动画实例中的曲线信息
	float GetAnimCurvesValue(FName CurvesName);

public:	

	/*
	* References
	*/

	UPROPERTY()
	class UALSCharacterAnimInstance* MainAnimInstance; //角色动画实例

	UPROPERTY()
	class APlayerController* PlayerController; //玩家控制器


	/*
	* PlayerInput
	*/

	UPROPERTY()
	class UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem; //增强输入本地玩家子系统

	UPROPERTY()
	class UEnhancedInputComponent* EnhancedInputComponent; 	//增强输入组件

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<class UInputMappingContext> InputMappingContext;	//输入映射上下文

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_ForwardMovementAction; //向前/后移动

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_RightMovementAction; //向右/左移动

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_LookUpAction; //向上/下移动视口

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_LookRightAction; //向右/左移动视口

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_JumpAction; //跳跃

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_WalkAction; //行走

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_SprintAction; //冲刺

	UPROPERTY(EditDefaultsOnly, Category = "ALS|EnhancedInput", BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_CrouchAction; //蹲伏


	/*
	* EssentialInformation
	*/

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	float Speed; //角色当前速度大小

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	FVector Acceleration; //角色当前加速度变化量

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	bool bIsMoving; //角色当前是否在移动

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	bool bHasMovementInput; //是否有运动输入

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	FRotator LastVelocityRotation; //下一帧的速度方向的旋转值

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	FRotator LastAccelerationRotation; //下一帧的加速度方向的旋转值

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	float MovementInputAmount; //有运动输入则大于0无运动输入则为0（0-1）

	UPROPERTY(VisibleAnywhere, Category = "ALS|EssentialInformation")
	float AimYawRate; //控制器每一帧旋转的速度大小


	/*
	* CacheVariables
	*/

	UPROPERTY(VisibleAnywhere, Category = "ALS|CacheVariables")
	FVector PreviousVelocity; //上一帧的加速度变化量

	UPROPERTY(VisibleAnywhere, Category = "ALS|CacheVariables")
	float PreviousAimYaw; //上一帧的控制器旋转的速度变化量


	/*
	* StateValues
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSMovementState MovementState; //角色当前运动状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSMovementState PrevMovementState; //角色上一时刻运动状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSMovementAction MovementAction; //角色蒙太奇

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSRotationMode RotationMode; //角色旋转模式

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSGait Gait; //角色步态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSStance Stance = EALSStance::ES_Standing; //角色姿态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSViewMode ViewMode =EALSViewMode::EVM_ThirdPerson; //角色视角模式

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|StateValues")
	EALSOverlayState OverlayState = EALSOverlayState::EOS_Default;  //角色叠加状态

	/*
	* Input
	*/
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Input")
	float LookUpDownRate = 1.25; //视口上下移动播放率

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Input")
	float LookRightLeftRate = 1.25; //视口左右移动播放率

	EALSGait DesiredGait = EALSGait::EG_Running; //期望的步态

	EALSRotationMode DesiredRotationMode = EALSRotationMode::ERM_LookingDirection; //期望的旋转模式

	EALSStance DesiredStance = EALSStance::ES_Standing;  //期望的角色姿态

	float LastStanceInputTime = 0.0f; //第一次按下蹲伏键的时间

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Input")
	float RollDoubleTapTimeout = 0.3f; //按下双击翻滚的最大间隔时间
	

	/*
	* CameraSystem
	*/

	UPROPERTY(EditDefaultsOnly, Category = "ALS|CameraSystem")
	bool bRightShoulder = true; //判断摄像机是否位于右肩

	UPROPERTY(EditDefaultsOnly, Category = "ALS|CameraSystem")
	float ThirdPersonFOV = 90.f; //第三人称视角大小

	UPROPERTY(EditDefaultsOnly, Category = "ALS|CameraSystem")
	float FirstPersonFOV = 90.f; //第一人称视角大小

	/*
	* MovementSystem
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|MovementSystem")
	FDataTableRowHandle MovementModel; //移动状态下参数的数据表

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|MovementSystem")
	FMovementSettingsState MovementData; //移动状态结构体

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|MovementSystem")
	FMovementSettings CurrentMovementSettings; //角色当前移动设置结构体

	/*
	* RotationSystem
	*/

	UPROPERTY(VisibleAnywhere, Category = "ALS|RotationSystem")
	FRotator TargetRotation; //人物当前旋转值

	UPROPERTY(VisibleAnywhere, Category = "ALS|RotationSystem")
	FRotator InAirRotation; //位于空中时的旋转

	/*
	* MantleSystem
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FMantleTraceSettings FallingTraceSettings; //跳跃状态下攀爬检测所需参数结构体

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FMantleTraceSettings GroundedTraceSettings; //地面状态下攀爬检测所需的参数结构体

	UPROPERTY(VisibleAnywhere, Category = "ALS|MantleSystem")
	FVector InitalTrace_ImpactPoint; //胶囊体检测击中墙体的第一个点

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FVector InitalTrace_Normal; //胶囊体检测击中墙体的法线

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FVector DownTraceLocation; //墙体顶部的接触面位置
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	class UPrimitiveComponent* HitComponent; //检测击中的基元组件

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FTransform TargetTransform; //墙体顶部胶囊体能够容纳后的锚点位置信息

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	float MantleHeight; //攀爬对应墙体的高度

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	EALSMantleType MantleType; //攀爬墙体对应的状态

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FMantleParams MantleParams; //攀爬所需的基础参数值

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FALSComponentAndTransform MantleLedgeLS; //角色局部空间时的击中返回组件和变换

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FTransform MantleTarget; //角色世界坐标下的攀爬变换 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FTransform MantleActualStartOffset; //攀爬角色实际的偏移量

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	FTransform MantleAnimatedStartOffset; //攀爬角色动画的偏移量

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|MantleSystem")
	TObjectPtr<class UTimelineComponent> MantleTimeline = nullptr; //攀爬时间轴，用于执行Updata，End

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|MantleSystem")
	TObjectPtr<UCurveFloat> MantleTimelineCurve; //攀爬时间轴所需的曲线


	/*
	* Roll
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Roll")
	bool bBreakfall; //判断角色着陆时的翻滚布尔值



	/*
	* AnimInstanceDebug
	*/



	/*
	* Timer
	*/

	FTimerHandle OnLandedTimer; //落地定时器句柄

	FTimerHandle BreakfallTimer; //落地翻滚定时器句柄
};