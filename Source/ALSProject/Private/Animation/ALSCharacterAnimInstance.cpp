#include "Animation/ALSCharacterAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Character/ALSBaseCharacter.h"
#include "Interface/ALSCharacterInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void UALSCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (IsValid(TryGetPawnOwner()))
	{
		ALSCharacter = Cast<ACharacter>(TryGetPawnOwner());
		//qwqwqq
		ALSBaseCharacter = Cast<AALSBaseCharacter>(ALSCharacter);
	}
}

void UALSCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	DeltaTimeX = DeltaTime;
	if (!ALSCharacter || DeltaTime == 0.0f)
	{
		return;
	}
	UpdateCharacterInformation();
	UpdateAimingValue();
	UpdateLayerValues();
	UpdateFootIK();
	if (MovementState == EALSMovementState::EMS_Grounded)
	{
		const bool bPrevShouldMove = bShouldMove;
		bShouldMove = ShouldMoveCheck();
		/*
		* 先判断当前时刻的ShouldMove是否为真
		* 若为真再进行下一步
		* 判断上一时刻的ShouldMove是否为假（代表上一时刻角色没有任何移动输入）
		* 若为假，则走ChangedToTrue（改变至真）
		* 
		* 若当前ShouldMove为假且上一时刻的ShouldMove为真
		* 则代表上一时刻有运动输入，而当前时刻没有
		* 则走Changed To Flase
		* 
		* 类似蓝图中的ML_DoWhile这个宏
		*/
		if (bShouldMove && bPrevShouldMove == false ) //Changed To True
		{
			ElapsedDelayTime = 0.0;
			bRotationL = false;
			bRotationR = false;
		}
		else //Changed To Flase
		{

		}
		/*
		* 若上一步判断为假，则进行下一步判断
		* 判断当前角色是否一直处于移动状态
		* 若为真，则走WhileTrue（一直为真）
		* 
		* 若ShouldMove判断为假
		* 则当前角色不处于移动状态
		* 则走WhileFalse
		* 
		* 类似蓝图中的ML_DoWhile这个宏
		*/
		if (bShouldMove) //While True
		{
			UpdateMovementValues();
			UpdateRotationValues();


		}
		else //While False
		{

			//Then0

			//是否可以进行原地旋转（第一人称或者瞄准状态下）
			if (CanRotateInPlace())
			{
				RotationInPlaceCheck();

			}
			else
			{
				bRotationL = false;
				bRotationR = false;
			}


			//Then1

			//是否可以进行原地转向（第三人称并且视口方向并且曲线值大于0.99）
			if (CanTurnInPlace())
			{
				TurnInPlaceCheck();
			}
			else
			{
				ElapsedDelayTime = 0.0;
			}


			//Then2

			//是否可以进行动态过渡（曲线值等于1）
			if (CanDynamicTransition())
			{
				DynamicTransitionCheck();
			}


		}
	}
	else if (MovementState == EALSMovementState::EMS_InAir)
	{
		UpdateInAirValue();
	}



	/*
	* Debug
	*/
	


}

void UALSCharacterAnimInstance::OnPivotDelay()
{
	bPivot = false;
}

void UALSCharacterAnimInstance::OnJumpedDelay()
{
	bJumped = false;
}

void UALSCharacterAnimInstance::PlayDynamicTransitionDelay()
{
	bCanPlayDynamicTransition = true;
}

void UALSCharacterAnimInstance::Jumped()
{



	bJumped = true;

	//根据人物的速度大小映射跳跃的播放率
	JumpPlayRate = UKismetMathLibrary::MapRangeClamped(
		Speed,
		0.0,
		600.0,
		1.2,
		1.5
	);

	GetWorld()->GetTimerManager().SetTimer(OnJumpedTimer, this,
		&UALSCharacterAnimInstance::OnJumpedDelay, 0.1f, false);

}

void UALSCharacterAnimInstance::SetGroundedEntryState(EALSGroundedEntryState LocalGroundedEntryState)
{
	GroundedEntryState = LocalGroundedEntryState;
}

void UALSCharacterAnimInstance::SetOverlayOverrideState(int32 LocalOverlayOverrideState)
{
	OverlayOverrideState = LocalOverlayOverrideState;
}

void UALSCharacterAnimInstance::AnimNotify_HipsF()
{
	TrackedHipsDirection = EALSHipsDirection::EHD_F;
}

void UALSCharacterAnimInstance::AnimNotify_HipsB()
{
	TrackedHipsDirection = EALSHipsDirection::EHD_B;
}

void UALSCharacterAnimInstance::AnimNotify_HipsLF()
{
	TrackedHipsDirection = EALSHipsDirection::EHD_LF;
}

void UALSCharacterAnimInstance::AnimNotify_HipsLB()
{
	TrackedHipsDirection = EALSHipsDirection::EHD_LB;
}

void UALSCharacterAnimInstance::AnimNotify_HipsRF()
{
	TrackedHipsDirection = EALSHipsDirection::EHD_RF;
}

void UALSCharacterAnimInstance::AnimNotify_HipsRB()
{
	TrackedHipsDirection = EALSHipsDirection::EHD_RB;
}

void UALSCharacterAnimInstance::AnimNotify_Pivot()
{
	bPivot = Speed < TriggerPivotSpeedLimit;
	GetWorld()->GetTimerManager().SetTimer(OnPivotTimer, this,
		&UALSCharacterAnimInstance::OnPivotDelay, 0.1f, false);
}

void UALSCharacterAnimInstance::AnimNotify_NStopL()
{
	FDynamicMontageParams DynamicMontageParams;
	DynamicMontageParams.Animation = LeftStopDownAnimation;
	DynamicMontageParams.BlendInTime = 0.2;
	DynamicMontageParams.BlendOutTime = 0.2;
	DynamicMontageParams.PlayRate = 1.5;
	DynamicMontageParams.StartTime = 0.4;
	PlayTransition(DynamicMontageParams);
}

void UALSCharacterAnimInstance::AnimNotify_NStopR()
{
	FDynamicMontageParams DynamicMontageParams;
	DynamicMontageParams.Animation = RightStopDownAnimation;
	DynamicMontageParams.BlendInTime = 0.2;
	DynamicMontageParams.BlendOutTime = 0.2;
	DynamicMontageParams.PlayRate = 1.5;
	DynamicMontageParams.StartTime = 0.4;
	PlayTransition(DynamicMontageParams);
}

void UALSCharacterAnimInstance::AnimNotify_StopTransition()
{

	StopSlotAnimation(0.2, FName("GroundedSlot"));
	StopSlotAnimation(0.2, FName("(N)Turn/Rotate"));
	StopSlotAnimation(0.2, FName("(CLF)Turn/Rotate"));

}

void UALSCharacterAnimInstance::AnimNotify_Land_Idle()
{
	FDynamicMontageParams DynamicMontageParams;
	DynamicMontageParams.Animation = Land_IdleTransitionAnimation;
	DynamicMontageParams.BlendInTime = 0.1;
	DynamicMontageParams.BlendOutTime = 0.2;
	DynamicMontageParams.PlayRate = 1.4;
	DynamicMontageParams.StartTime = 0.0;
	PlayTransition(DynamicMontageParams);
}

void UALSCharacterAnimInstance::AnimNotify_CLFStop()
{
	FDynamicMontageParams DynamicMontageParams;
	DynamicMontageParams.Animation = CLFStopTransitionAnimation;
	DynamicMontageParams.BlendInTime = 0.2;
	DynamicMontageParams.BlendOutTime = 0.2;
	DynamicMontageParams.PlayRate = 1.5;
	DynamicMontageParams.StartTime = 0.2;
	PlayTransition(DynamicMontageParams);
}

void UALSCharacterAnimInstance::AnimNotify_NQuickStop()
{
	FDynamicMontageParams DynamicMontageParams;
	DynamicMontageParams.Animation = NQuickStopTransitionAnimation;
	DynamicMontageParams.BlendInTime = 0.1;
	DynamicMontageParams.BlendOutTime = 0.2;
	DynamicMontageParams.PlayRate = 1.75;
	DynamicMontageParams.StartTime = 0.3;
	PlayTransition(DynamicMontageParams);
}

void UALSCharacterAnimInstance::AnimNotify_Roll_Idle()
{
	FDynamicMontageParams DynamicMontageParams;
	DynamicMontageParams.Animation = Roll_IdleTransitionAnimation;
	DynamicMontageParams.BlendInTime = 0.2;
	DynamicMontageParams.BlendOutTime = 0.2;
	DynamicMontageParams.PlayRate = 1.5;
	DynamicMontageParams.StartTime = 0.2;
	PlayTransition(DynamicMontageParams);
}

void UALSCharacterAnimInstance::AnimNotify_Reset_GroundedEntryState()
{
	GroundedEntryState = EALSGroundedEntryState::EES_None;
}

void UALSCharacterAnimInstance::UpdateCharacterInformation()
{
	IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(ALSCharacter);

	//Then0

	ALSCharacterInterface->GetEssentialValues(
		Velocity,
		Acceleration,
		MovementInput,
		bIsMoving,
		bHasMovingInput,
		Speed,
		MovementInputAmount,
		AimYawRate,
		AimingRotation
	);

	//Then1

	ALSCharacterInterface->GetCurrentState(
		MovementMode,
		MovementState,
		PrevMovementState,
		MovementAction,
		RotationMode,
		Gait,
		Stance,
		ViewMode,
		OverlayState
	);

}

void UALSCharacterAnimInstance::UpdateMovementValues()
{

	//Then0
	//利用插值过渡的方式设置六向动画的速度的混合

	VelocityBlend = InterpVelocityBlend(
		VelocityBlend,
		CalculateVelocityBlend(),
		VelocityBlendInterpSpeed,
		DeltaTimeX
	);

	//Then1
	//计算脚步对角线上的混合

	DiagonalScaleAmount = CalculateDiagonalAmount();

	//Then2
	//用加速度计算偏移量
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();

	FLeanAmount LocLeanAmount;
	LocLeanAmount.FB = RelativeAccelerationAmount.X;
	LocLeanAmount.LR = RelativeAccelerationAmount.Y;

	LeanAmount = InterpLeanAmount(
		LeanAmount,
		LocLeanAmount,
		GroundedLeanInterpSpeed,
		DeltaTimeX
	);

	//Then3
	//计算动画需要的数值
	WalkRunBlend = CalculateWalkRunBlend();
	StrideBlend = CalculateStrideBlend();
	StandPlayRate = CalculateStandingPlayRate();
	CrouchPlayRate = CalculateCrouchingPlayRate();
}

void UALSCharacterAnimInstance::UpdateRotationValues()
{

	/*
	* 此处计算的角色旋转值和角色蓝图中不同的是，此处的数值是使用于状态机当中，不会改变人物的旋转值
	*/


	//Then0

	MovementDirection = CalculateMovementDirection();




	//Then1

	/*
	* 由于摄像机的位置有差异，并不是位于人物正后方
	* 所以需要个偏移值来修正
	* 计算出移动时人物的偏移量，主要在动画实例中赋值给YawOffset后应用于人物中
	* 此处计算的YawOffset值将运用与状态机中六向状态机修改曲线中
	* 被修改的曲线为YawOffset
	* 该曲线应用与ALSBaseCharacter中的SmoothCharacterRotation中
	* 将角色控制旋转的Yaw值和曲线值进行相加，然后再进行插值过渡的方式
	* 实时更新角色的旋转值SetActorRotation
	*/
	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(Velocity.ToOrientationRotator(), ALSCharacter->GetControlRotation());
	
	/*
	* XY映射曲线相同
	* 0 - 60度映射为0 - 60偏移量
	* 60 - 90度映射为60 - 0偏移量
	* 90 - 120度映射为0 - -60偏移量
	* 120 - 180度映射为-60 - 0偏移量
	* 负轴是对称相反
	*/
	FVector FBVector = YawOffsetFB->GetVectorValue(DeltaRotation.Yaw);
	/*
	* 
	*/
	FVector LRVector = YawOffsetLR->GetVectorValue(DeltaRotation.Yaw);

	FYaw = FBVector.X;
	BYaw = FBVector.Y;
	LYaw = LRVector.X;
	RYaw = LRVector.Y;

}

void UALSCharacterAnimInstance::UpdateAimingValue()
{

	//Then0

	//将控制器旋转值以平滑过渡的形式同步到SmoothAimingRotation
	SmoothedAimingRotation = UKismetMathLibrary::RInterpTo(
		SmoothedAimingRotation,
		AimingRotation,
		DeltaTimeX,
		SmoothedAimingRotationInterpSpeed
	);


	//Then1

	//计算控制器旋转值和Actor锚点旋转值之间的差量
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(
		AimingRotation,
		ALSCharacter->GetActorRotation()
	);
	//然后将差量 Yaw（左右）,Pitch（上下） 赋值给AimingAngle的 X，Y
	AimingAngle = FVector2D(DeltaRotator.Yaw,DeltaRotator.Pitch);

	//计算平滑插值控制器旋转过渡值和Actor锚点旋转值之间的差量
	FRotator DeltaSmoothedRotator = UKismetMathLibrary::NormalizedDeltaRotator(
		SmoothedAimingRotation,
		ALSCharacter->GetActorRotation()
	);
	//然后将差量 Yaw（左右）,Pitch（上下） 赋值给SmoothedAimingAngle的 X，Y
	SmoothedAimingAngle = FVector2D(DeltaSmoothedRotator.Yaw, DeltaSmoothedRotator.Pitch);

	//Then2

	if (RotationMode == EALSRotationMode::ERM_LookingDirection ||
		RotationMode == EALSRotationMode::ERM_Aiming)
	{
		AImSweepTime = UKismetMathLibrary::MapRangeClamped(
			AimingAngle.Y,
			-90.0,
			90.0,
			1.0,
			0.0
		);
	}

	SpineRotation = FRotator(0.0, AimingAngle.X / 4.0, 0.0);

	//Then3

	if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
	{
		if (bHasMovingInput)
		{
			InputYawOffsetTime = UKismetMathLibrary::FInterpTo(
				InputYawOffsetTime,
				UKismetMathLibrary::MapRangeClamped(
					UKismetMathLibrary::NormalizedDeltaRotator(
						MovementInput.ToOrientationRotator(),
						ALSBaseCharacter->GetActorRotation()
					).Yaw,
					-180.0,
					180.0,
					0.0,
					1.0
				),
				DeltaTimeX,
				InputYawOffsetInterpSpeed
				);
		}
	}

	//Then4

	LeftYawTime = UKismetMathLibrary::MapRangeClamped(
		UKismetMathLibrary::Abs(SmoothedAimingAngle.X),
		0.0,
		180.0,
		0.5,
		0.0
	);

	RightYawTime = UKismetMathLibrary::MapRangeClamped(
		UKismetMathLibrary::Abs(SmoothedAimingAngle.X),
		0.0,
		180.0,
		0.5,
		1.0
	);

	ForwardYawTime = UKismetMathLibrary::MapRangeClamped(
		SmoothedAimingAngle.X,
		-180.0,
		180.0,
		0.0,
		1.0
	);

}

void UALSCharacterAnimInstance::UpdateLayerValues()
{

	//Then0

	EnableAimOffset = UKismetMathLibrary::Lerp(
		1.0,
		0.0,
		GetAnimCurveCompact(FName("Mask_AimOffset"))
	);

	//Then1

	BasePoseN = GetAnimCurveCompact(FName("Basepose_N"));

	BasePoseCLF = GetAnimCurveCompact(FName("Basepose_CLF"));
		
	//Then2

	LayeringSpineAdd = GetAnimCurveCompact(FName("Layering_Spine_Add"));

	LayeringHeadAdd = GetAnimCurveCompact(FName("Layering_Head_Add"));
	
	LayeringArmLAdd = GetAnimCurveCompact(FName("Layering_Arm_L_Add"));

	LayeringArmRAdd = GetAnimCurveCompact(FName("Layering_Arm_R_Add"));
	
	//Then3

	LayeringHandL = GetAnimCurveCompact(FName("Layering_Hand_L"));

	LayeringHandR = GetAnimCurveCompact(FName("Layering_Hand_R"));

	//Then4

	EnableHandIKL = UKismetMathLibrary::Lerp(
		0.0,
		GetAnimCurveCompact(FName("Enable_HandIK_L")),
		GetAnimCurveCompact(FName("Layering_Arm_L"))
	);

	EnableHandIKR = UKismetMathLibrary::Lerp(
		0.0,
		GetAnimCurveCompact(FName("Enable_HandIK_R")),
		GetAnimCurveCompact(FName("Layering_Arm_R"))
	);

	//Then5

	ArmLLS = GetAnimCurveCompact(FName("Layering_Arm_L_LS"));

	ArmRLS = GetAnimCurveCompact(FName("Layering_Arm_R_LS"));

	ArmLMS = (1 - UKismetMathLibrary::FFloor(ArmLLS));

	ArmRMS = (1 - UKismetMathLibrary::FFloor(ArmRLS));

}

void UALSCharacterAnimInstance::UpdateFootIK()
{

	FVector FootOffset_L_Target = FVector::ZeroVector;

	FVector FootOffset_R_Target = FVector::ZeroVector;

	/*
	* 设置左脚基于root的骨骼空间的脚步锁定
	* Enable_FootIK_L 启用左脚脚步IK骨骼曲线（当处于地面状态时，该曲线数值为1，反之为0）
	* FootLock_L 角色左脚的脚步锁定曲线信息,在动画序列中修改该骨骼曲线的数值
	* ik_foot_l 角色的左脚脚步IK骨骼命名，用于获取骨骼空间中角色IK骨骼的变换信息
	* FootLock_L_Alpha 通过引用的方式将函数中计算的角色左脚的锁定曲线值（动画序列中修改）传递给FootLock_L_Alpha
	* FootLock_L_Location 通过引用的方式将函数中计算的角色左脚的锁定Location值传递给FootLock_L_Alpha
	* FootLock_L_Rotation 通过引用的方式将函数中计算的角色左脚的锁定Rotation值传递给FootLock_L_Alpha
	*/
	SetFootLocking
	(
		FName("Enable_FootIK_L"),
		FName("FootLock_L"),
		FName("ik_foot_l"),
		FootLock_L_Alpha,
		FootLock_L_Location,
		FootLock_L_Rotation
	);

	/*
	* 设置右脚基于root的骨骼空间的脚步锁定
	* Enable_FootIK_R 启用右脚脚步IK骨骼曲线（当处于地面状态时，该曲线数值为1，反之为0）
	* FootLock_R 角色右脚的脚步锁定曲线信息,在动画序列中修改该骨骼曲线的数值
	* ik_foot_r 角色的右脚脚步IK骨骼命名，用于获取骨骼空间中角色IK骨骼的变换信息
	* FootLock_R_Alpha 通过引用的方式将函数中计算的角色右脚的锁定的曲线值（动画序列中修改）传递给FootLock_R_Alpha
	* FootLock_R_Location 通过引用的方式将函数中计算的角色右脚的锁定Location值传递给FootLock_R_Alpha
	* FootLock_R_Rotation 通过引用的方式将函数中计算的角色右脚的锁定Rotation值传递给FootLock_R_Alpha
	*/
	SetFootLocking
	(
		FName("Enable_FootIK_R"),
		FName("FootLock_R"),
		FName("ik_foot_r"),
		FootLock_R_Alpha,
		FootLock_R_Location,
		FootLock_R_Rotation
	);

	//当角色不处于空中状态或者布娃娃状态
	if (MovementState == EALSMovementState::EMS_None ||
		MovementState == EALSMovementState::EMS_Grounded ||
		MovementState == EALSMovementState::EMS_Mantling
		)
	{
		
		SetFootOffsets(
			FName("Enable_FootIK_L"),
			FName("ik_foot_l"),
			FName("root"),
			FootOffset_L_Target,
			FootOffset_L_Location,
			FootOffset_L_Rotation
		);
		
		SetFootOffsets(
			FName("Enable_FootIK_R"),
			FName("ik_foot_r"),
			FName("root"),
			FootOffset_R_Target,
			FootOffset_R_Location,
			FootOffset_R_Rotation
		);

		SetPeivisIKOffset(FootOffset_L_Target, FootOffset_R_Target);

	}
	else if(MovementState == EALSMovementState::EMS_InAir)
	{
		SetPeivisIKOffset(FVector(0.0, 0.0, 0.0), FVector(0.0, 0.0, 0.0));
		ResetIKOffset();
	}
		


}

void UALSCharacterAnimInstance::UpdateInAirValue()
{

	//Then0

	/*
	* 计算下落速度
	*/


	FallSpeed = Velocity.Z;


	//Then1

	/*
	* 处理落地的效果
	*/

	LandPrediction = CalculateLandPrediction();


	//Then2

	/*
	* 处理空中的偏移量
	*/

	LeanAmount = InterpLeanAmount(
		LeanAmount,
		CalculateInAirLeanAmount(),
		InAirLeanInterpSpeed,
		DeltaTimeX
	);


}

bool UALSCharacterAnimInstance::ShouldMoveCheck()
{
	//检测角色当前是否进行移动
	return (bHasMovingInput && bIsMoving) || Speed > 150.0f;
}

bool UALSCharacterAnimInstance::CanRotateInPlace()
{
	//判断角色是否处于瞄准状态或者第一人称状态
	return RotationMode == EALSRotationMode::ERM_Aiming || ViewMode == EALSViewMode::EVM_FirstPerson;
	
}

bool UALSCharacterAnimInstance::CanTurnInPlace()
{
	//判断角色是否处于视口方向并且是否处于第三人称状态并且Enable_Transition曲线值是否大于0.99
	return RotationMode == EALSRotationMode::ERM_LookingDirection &&
		ViewMode == EALSViewMode::EVM_ThirdPerson &&
		GetCurveValue(FName("Enable_Transition")) > 0.99;

}

bool UALSCharacterAnimInstance::CanDynamicTransition()
{
	//判断曲线值Enable_Transition是否等于1
	return GetCurveValue(FName("Enable_Transition")) == 1;

}

void UALSCharacterAnimInstance::TurnInPlaceCheck()
{
	//判断人物是否可以原地转向（人物控制旋转和Actor旋转Yaw的差值>45度 && 人物控制器旋转值大小是否小于50)
	if (UKismetMathLibrary::Abs(AimingAngle.X) > TurnCheckMinAngle &&
		AimYawRate < AimYawRateLimit)
	{
		//延迟时间与帧率进行叠加
		ElapsedDelayTime = ElapsedDelayTime + DeltaTimeX;
		//判断叠加的帧率是否大于根据旋转角度进行限制的延迟时间，若大于则执行下一步（45 - 180    0.75 - 0）角度越大所需的延迟时间越短，角度越小所需的延迟时间越长
		if (ElapsedDelayTime > UKismetMathLibrary::MapRangeClamped(
			UKismetMathLibrary::Abs(AimingAngle.X),
			TurnCheckMinAngle,
			180.0,
			MinAngleDelay,
			MaxAngleDelay
		)	
			)
		{
			TurnInPlace(
				FRotator(0.0, AimingRotation.Yaw, 0.0),
				1.0,
				0.0,
				false
			);
		}

	}
	else
	{

		ElapsedDelayTime = 0;

	}

}

void UALSCharacterAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent)
{

	//Then0

	//实时获取控制器和人物旋转值差量的Yaw值，该值为旋转的角度
	float TurnAngle = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, ALSCharacter->GetActorRotation()).Yaw;


	//Then1

	//最终将编辑器中初始化好的数值根据枚举信息的不同赋值到该结构体中
	FTurnInPlaceAsset TargetTurnAsset;

	//判断旋转角度的绝对值是否小于130度，若小于则执行90度的结构体参数值，若大于则执行180的结构体参数值
	if (UKismetMathLibrary::Abs(TurnAngle) < Turn180Threshold)
	{
		//判断旋转角度是否小于0，小于0则角色是向左旋转，若大于0则角色是向右旋转
		if (TurnAngle < 0)
		{
			//判断角色是否处于站立状态，反之则为蹲伏状态
			if (Stance == EALSStance::ES_Standing)
			{
				TargetTurnAsset = N_TurnIP_L90;
			}
			else
			{
				TargetTurnAsset = CLF_TurnIP_L90;
			}
		}
		else
		{
			//判断角色是否处于站立状态，反之则为蹲伏状态
			if (Stance == EALSStance::ES_Standing)
			{
				TargetTurnAsset = N_TurnIP_R90;
			}
			else
			{
				TargetTurnAsset = CLF_TurnIP_R90;
			}
		}
	}
	else
	{
		//判断旋转角度是否小于0，小于0则角色是向左旋转，若大于0则角色是向右旋转
		if (TurnAngle < 0)
		{
			//判断角色是否处于站立状态，反之则为蹲伏状态
			if (Stance == EALSStance::ES_Standing)
			{
				TargetTurnAsset = N_TurnIP_L180;
			}
			else
			{
				TargetTurnAsset = CLF_TurnIP_L180;
			}
		}
		else
		{
			//判断角色是否处于站立状态，反之则为蹲伏状态
			if (Stance == EALSStance::ES_Standing)
			{
				TargetTurnAsset = N_TurnIP_R180;
			}
			else
			{
				TargetTurnAsset = CLF_TurnIP_R180;
			}
		}
	}

	//Then2

	//判断该动画是否正在播放插槽动画（蒙太奇动画），若非正在播放，才可以继续执行(IsPlayingSlotAnimation函数为true则为正在播放，false则为不在播放）不在播放时，才可以继续执行
	if (OverrideCurrent || !IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		//以此创建一个动态的UAnimMontage，在动画插槽中播放一个动画序列非蒙太奇动画，且每个插槽中仅只能播放一种蒙太奇动画或动画序列
		PlaySlotAnimationAsDynamicMontage(
			TargetTurnAsset.Animation,
			TargetTurnAsset.SlotName,
			0.2,
			0.2,
			TargetTurnAsset.PlayRate * PlayRateScale,
			1,
			0.0,
			StartTime
		);
		//判断是站立转向还是蹲伏转向，true站立，false蹲伏
		if (TargetTurnAsset.ScaleTurnAngle)
		{
			//计算站立旋转缩放量,该缩放值应用于动画蓝图的修改曲线中
			//（旋转角度/动画角度(-180，-90，90，180)） *  PlayRate(1.2) * PlayRateScale(1)
			RotationScale = (TurnAngle / TargetTurnAsset.AnimatedAngle) * TargetTurnAsset.PlayRate * PlayRateScale;
		}
		else
		{
			//计算蹲伏旋转缩放量,该缩放值应用于动画蓝图的修改曲线中
			//PlayRate(1.2)* PlayRateScale(1)
			RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
		}

	}

}

void UALSCharacterAnimInstance::RotationInPlaceCheck()
{

	//Then0

	//若旋转角度小于旋转所需的最小值（-50）则左向为真
	bRotationL = AimingAngle.X < RotateMinThreshold;

	//若旋转角度大于旋转所需的最大值（50）则右向为真
	bRotationR = AimingAngle.X > RotateMaxThreshold;

	//Then1
	
	//若任意一个布尔值为真，才可以执行原地旋转
	if (bRotationL || bRotationR)
	{
		//将控制器Yaw值旋转速率映射到人物旋转上(90 - 270    1.15 - 3)
		RotateRate = UKismetMathLibrary::MapRangeClamped(
			AimYawRate,
			AimYawRateMinRange,
			AimYawRateMaxRange,
			MinPlayRate,
			MaxPlayRate
		);
	}

}

void UALSCharacterAnimInstance::DynamicTransitionCheck()
{

	//Then0


	/*
	* 获取IK骨骼ik_foot_l与虚拟骨骼VB foot_target_l的骨骼空间中的变换信息
	* 然后将二者作差量计算，计算两个骨骼变换之间的距离大小（初始值为0，不进行Transition则Distance为0）
	*/


	FTransform LeftSocketTransformA = GetOwningComponent()->GetSocketTransform(
		FName("ik_foot_l"), ERelativeTransformSpace::RTS_Component);
	FTransform LeftSocketTransformB = GetOwningComponent()->GetSocketTransform(
		FName("VB foot_target_l"), ERelativeTransformSpace::RTS_Component);
	float LeftDistance = (LeftSocketTransformB.GetLocation() - LeftSocketTransformA.GetLocation()).Size();

	if (LeftDistance > 8.0)
	{
		FDynamicMontageParams DynamicMontageParams;
		DynamicMontageParams.Animation = RightTransitionAnimation;
		DynamicMontageParams.BlendInTime = 0.2;
		DynamicMontageParams.BlendOutTime = 0.2;
		DynamicMontageParams.PlayRate = 1.5;
		DynamicMontageParams.StartTime = 0.8;
		PlayDynamicTransition
		(
			0.1,
			DynamicMontageParams
		);
	}

	//Then1

	FTransform RightSocketTransformA = GetOwningComponent()->GetSocketTransform(
		FName("ik_foot_r"), ERelativeTransformSpace::RTS_Component);
	FTransform RightSocketTransformB = GetOwningComponent()->GetSocketTransform(
		FName("VB foot_target_r"), ERelativeTransformSpace::RTS_Component);
	float RightDistance = (RightSocketTransformB.GetLocation() - RightSocketTransformA.GetLocation()).Size();

	if (RightDistance > 8.0)
	{
		
		FDynamicMontageParams DynamicMontageParams;
		DynamicMontageParams.Animation = LeftTransitionAnimation;
		DynamicMontageParams.BlendInTime = 0.2;
		DynamicMontageParams.BlendOutTime = 0.2;
		DynamicMontageParams.PlayRate = 1.5;
		DynamicMontageParams.StartTime = 0.8;
		PlayDynamicTransition
		(
			0.1,
			DynamicMontageParams
		);

	}


}

void UALSCharacterAnimInstance::PlayTransition(FDynamicMontageParams Parameters)
{

	/*
	* 
	*/

	PlaySlotAnimationAsDynamicMontage(
		Parameters.Animation,
		FName("GroundedSlot"),
		Parameters.BlendInTime,
		Parameters.BlendOutTime,
		Parameters.PlayRate,
		1,
		0.0,
		Parameters.StartTime
	);


}

void UALSCharacterAnimInstance::PlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters)
{

	if (bCanPlayDynamicTransition)
	{

		bCanPlayDynamicTransition = false;

		PlayTransition(Parameters);

		UWorld* World = GetWorld();
		check(World);
		World->GetTimerManager().SetTimer(PlayDynamicTransitionTimer, this,
			&UALSCharacterAnimInstance::PlayDynamicTransitionDelay,
			ReTriggerDelay, false);

	}


}

float UALSCharacterAnimInstance::CalculateLandPrediction()
{
	
	if (FallSpeed < -200.0)
	{

		FVector Start = ALSCharacter->GetCapsuleComponent()->GetComponentLocation();

		FVector VelocityNormal = Velocity;
		VelocityNormal.Z = UKismetMathLibrary::Clamp(Velocity.Z, -4000, -200);
		VelocityNormal.Normalize();
		FVector End = Start +
			(VelocityNormal * UKismetMathLibrary::MapRangeClamped(
				Velocity.Z,
				0.0,
				-4000.0,
				50.0,
				2000.0)
			);

		FHitResult HitResult;
		TArray<AActor*>ActorToIgnore;
		UKismetSystemLibrary::CapsuleTraceSingleByProfile(
			this,
			Start,
			End,
			ALSCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
			ALSCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(),
			FName("ALS_Character"),
			false,
			ActorToIgnore,
			EDrawDebugTrace::ForOneFrame,
			HitResult,
			true
		);

		if (ALSCharacter->GetCharacterMovement()->IsWalkable(HitResult) && HitResult.bBlockingHit)
		{
			
			return UKismetMathLibrary::Lerp(
				LandPredictionCurve->GetFloatValue(HitResult.Time),
				0.0,
				GetCurveValue(FName("Mask_LandPrediction"))
			);
		}
		else
		{
			return 0.0;
		}
	}
	else
	{
		return 0.0;
	}


}

FLeanAmount UALSCharacterAnimInstance::CalculateInAirLeanAmount()
{

	/*
	* 计算人物在空中的偏移量
	*/

	FLeanAmount ReturnLeanAmount;
	FVector2D UnRotatorVelocity;
	UnRotatorVelocity.X = (ALSCharacter->GetActorRotation().UnrotateVector(Velocity) / 350.0).Y;
	UnRotatorVelocity.Y = (ALSCharacter->GetActorRotation().UnrotateVector(Velocity) / 350.0).X;
	ReturnLeanAmount.LR = (UnRotatorVelocity * LeanInAirCurve->GetFloatValue(FallSpeed)).X;
	ReturnLeanAmount.FB = (UnRotatorVelocity * LeanInAirCurve->GetFloatValue(FallSpeed)).Y;
	return ReturnLeanAmount;


}

void UALSCharacterAnimInstance::SetFootLocking(FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone, float& CurrentFootLockAlpha, FVector& CurrentFootLockLocation, FRotator& CurrentFootLockRotation)
{
	//判断脚步锁定的曲线是否更改（使用的动画是否含有脚步锁定曲线）
	if (GetCurveValue(EnableFootIKCurve) > 0.0)
	{

		//Then0

		/*
		* 获取脚步锁定曲线中(动画序列中设置)的数值
		*/

		float FootLockCurveValue = GetCurveValue(FootLockCurve);

		//Then1

		/*
		* 判断脚步锁定是否开启或者曲线值是否大于当前脚步锁定的Alpha值
		* 将脚步锁定曲线值（动画序列中修改）按引用传递给CurrentFootLockAlpha
		* 同时按引用传递也会更改FootLock_L_Alpha或FootLock_R_Alpha
		*/
		if (FootLockCurveValue >= 0.99 ||
			FootLockCurveValue < CurrentFootLockAlpha
			)
		{
			CurrentFootLockAlpha = FootLockCurveValue;
		}


		//Then2

		/*
		* 判断脚步锁定曲线值是否大于0.99
		* 然后基于角色root骨骼的组件空间（root坐标信息为0，0，0）
		* 将IK骨骼(ik_foot_l)同时也是虚拟骨骼的位置和旋转信息按引用的方式传递给CurrentFootLockLocation和CurrentFootLockRotation
		* 同时按引用传递也会更改（FootLock_L_Location和FootLock_L_Rotation）或者（FootLock_R_Location和FootLock_R_Rotation）
		*/

		if (CurrentFootLockAlpha >= 0.99)
		{
			CurrentFootLockLocation = GetOwningComponent()->GetSocketTransform
			(
				IKFootBone, ERelativeTransformSpace::RTS_Component
			).GetLocation();

			CurrentFootLockRotation = GetOwningComponent()->GetSocketTransform
			(
				IKFootBone, ERelativeTransformSpace::RTS_Component
			).Rotator();
		}


		//Then3

		/*
		* 判断脚步锁定曲线是否大于0
		* 设置脚步锁定的偏移量
		* 同时将虚拟ik骨骼的Location和Rotation以引用的形式传进去
		*/

		if (CurrentFootLockAlpha > 0.0)
		{
			SetFootLockOffset(CurrentFootLockLocation, CurrentFootLockRotation);
		}

	}
	
}

void UALSCharacterAnimInstance::SetFootLockOffset(FVector& LocalLocation, FRotator& LocalRotation)
{

	//Then0

	FRotator RotationDifference;

	FVector LocationDifference;

	/*
	* 判断角色当前是否处于地面移动
	* 获取当前帧与上一帧之间的旋转差量（基于局部空间坐标系）
	*/



	if (ALSCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		RotationDifference = UKismetMathLibrary::NormalizedDeltaRotator
		(
			ALSCharacter->GetActorRotation(),
			ALSCharacter->GetCharacterMovement()->GetLastUpdateRotation()
		);
	}

	//Then1

	/*
	* 获取每一帧的角色各方向速度
	* 将其通过不旋转向量
	* 将角色速度从X轴转到Y轴
	* 也就是角色运动正向为Y轴，右向为X轴
	* Y轴符合骨骼空间中的角色朝向
	* 以此缩小长度来实现脚步锁定应该移动距离的值
	*/
	
	LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector
	(
		(Velocity * UGameplayStatics::GetWorldDeltaSeconds(this))
	);


	//Then2

	/*
	* 用ik骨骼（基于root）左脚或者右脚的位置信息来减去LocationDifference
	* 然后使用ik骨骼左脚或者右脚的旋转值信息来旋转该向量
	* 然后将其赋值给LocalLocation
	* 以此来获取脚步锁定实际上所应用的位置信息
	*/

	LocalLocation = UKismetMathLibrary::RotateAngleAxis
	(
		LocalLocation - LocationDifference,
		RotationDifference.Yaw,
		FVector(0.0, 0.0, -1.0)
	);


	//Then3

	/*
	* 使用ik骨骼(基于骨骼空间）左脚或者右脚的旋转值信息
	* 和旋转差值进行差值计算
	* 获取脚步锁定实际应用的旋转值
	*/

	LocalRotation = UKismetMathLibrary::NormalizedDeltaRotator
	(
		LocalRotation,
		RotationDifference
	);



}
void UALSCharacterAnimInstance::SetFootOffsets(FName EnableFootIKCurve, FName IKFootBone, FName RootBone, FVector& CurrentLocationTarget, FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset)
{
	//判断IK曲线是否开启
	if (GetCurveValue(EnableFootIKCurve) > 0.0)
	{

		//Then0

		/*
		* 获取角色地面状态下，脚步ik骨骼的位置信息( X:ik_foot_l/r.X , Y:ik_foot.Yl/r , Z:root.Z)
		* 最终位置是让ik骨骼的Z值与root面的Z值相同,X,Y位置不变
		* 然后使用射线检测根据脚步ik地面的位置信息Z轴正方向伸长100cm为Start
		* 然后根据脚步IK地面的位置信息Z轴负方向伸长45cm为End
		* 以此来检测脚步ik骨骼检测地面的信息
		*/

		FVector IKFootFloorLocation = FVector
		(
			GetOwningComponent()->GetSocketLocation(IKFootBone).X,
			GetOwningComponent()->GetSocketLocation(IKFootBone).Y,
			GetOwningComponent()->GetSocketLocation(RootBone).Z
		);

		FHitResult HitResult;
		TArray<AActor*>ActorsToIgnore;
		FVector Start = IKFootFloorLocation + FVector(0.0,0.0,IKTraceDistanceAboveFoot);
		FVector End = IKFootFloorLocation - FVector(0.0,0.0,IKTraceDistanceBelowFoot);
		UKismetSystemLibrary::LineTraceSingle
		(
			this,
			Start,
			End,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame,
			HitResult,
			true
		);
		FVector ImpactPoint;
		FVector ImpactNormal;
		FRotator TargetRotationOffset;

		//判断是否为可行走地面
		if (ALSCharacter->GetCharacterMovement()->IsWalkable(HitResult))
		{
			ImpactPoint = HitResult.ImpactPoint;
			ImpactNormal = HitResult.ImpactNormal;

			/*
			* 将击中点沿着法线方向提高13.5cm（法线时刻与地面相垂直)
			* 然后减去IKFootFloorLocation沿Z轴升高13.5cm的向量
			* 最终得到脚面距离地面的偏移量
			*/

			CurrentLocationTarget =
				(ImpactPoint + (ImpactNormal * FootHeight)) -
				(IKFootFloorLocation + FVector(0.0, 0.0, FootHeight));

			/*
			* 以30度斜坡为例
			* 若脚面踩到斜坡上，按照实际脚步则陷入斜坡里
			* 所以需要将脚面进行旋转
			* 使脚面不再陷入斜坡内
			* 此时的法线向量为 （ 1/2 ， 0  ， 根号3/2 ）
			* Pitch:arctan X/Z == arctan( 1/2 / 根号3/2) * -1 == 30度
			* Roll: arctan Y/Z == arctan( 0 / 根号3/2)        == 0度
			* 所以此处我们需要将脚面沿着Pitch旋转30度，沿着Rool旋转0度
			* 最终得到角色脚面位于斜坡时，从斜坡内旋转到斜坡外的旋转角度
			*/
			
			TargetRotationOffset = FRotator
			(
				FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.X, ImpactNormal.Z)) * -1.0,
				0.0,
				FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z))
			);
			
		}
		else
		{
			TargetRotationOffset = FRotator(0.0, 0.0, 0.0);
		}

		//Then1

		/*
		* 判断偏移值是否大于目标值
		* 若大于则以更高的插值速度来对偏移值到目标值进行插值过渡
		* 若小于则以更低的插值速度来对偏移值到目标值进行插值过渡
		*/

		if (CurrentLocationOffset.Z > CurrentLocationTarget.Z)
		{
			CurrentLocationOffset = UKismetMathLibrary::VInterpTo
			(
				CurrentLocationOffset,
				CurrentLocationTarget,
				UGameplayStatics::GetWorldDeltaSeconds(this),
				30.0
			);
		}
		else
		{
			CurrentLocationOffset = UKismetMathLibrary::VInterpTo
			(
				CurrentLocationOffset,
				CurrentLocationTarget,
				UGameplayStatics::GetWorldDeltaSeconds(this),
				15.0
			);
		}

		//Then2

		/*
		* 将当前的旋转偏移值插值到目标旋转偏移值
		* 再以引入的方式传回给当前旋转偏移值
		*/

		CurrentRotationOffset = UKismetMathLibrary::RInterpTo
		(
			CurrentRotationOffset,
			TargetRotationOffset,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			30.0
		);

	}
	else
	{
		CurrentLocationOffset = FVector(0.0, 0.0, 0.0);
		CurrentRotationOffset = FRotator(0.0, 0.0, 0.0);
	}


}

void UALSCharacterAnimInstance::SetPeivisIKOffset(FVector FootOffsetLTarget, FVector FootOffsetRTarget)
{

	/*
	* 判断角色整体IK动画曲线是否开启
	*/

	PelvisAlpha = (
		GetCurveValue(FName("Enable_FootIK_L")) + 
		GetCurveValue(FName("Enable_FootIK_R"))
		) / 2.0;

	FVector PelvisTarget;
	//若Alpha大于0，则左脚或者右脚的IK曲线已开启
	if (PelvisAlpha > 0.0)
	{

		//Then0

		/*
		* 判断角色左脚和右脚着地时，哪只脚的脚步偏移目标Z值更小
		* 若左脚在高处，右脚在低处，则右脚的偏移值小于左脚
		* 角色锚点位置会更靠低处
		*/
		if (FootOffsetLTarget.Z < FootOffsetRTarget.Z)
		{
			PelvisTarget = FootOffsetLTarget;
		}
		else
		{
			PelvisTarget = FootOffsetRTarget;
		}

		//Then1

		/*
		* 设置人物整体偏移位置
		* 判断IK脚是比root面更高或是更矮
		* 若目标值大于偏移值，则角色的Ik脚位于root面之上
		* 若目标值小于偏移值，则角色的Ik脚位于root面之下
		*/
		if (PelvisTarget.Z > PelvisOffset.Z)
		{
			PelvisOffset = UKismetMathLibrary::VInterpTo
			(
				PelvisOffset,
				PelvisTarget,
				UGameplayStatics::GetWorldDeltaSeconds(this),
				10.0
			);
		}
		else
		{
			PelvisOffset = UKismetMathLibrary::VInterpTo
			(
				PelvisOffset,
				PelvisTarget,
				UGameplayStatics::GetWorldDeltaSeconds(this),
				15.0
			);
		}
	}
	else
	{
		PelvisOffset = FVector(0.0, 0.0, 0.0);
	}


}

void UALSCharacterAnimInstance::ResetIKOffset()
{

	/*
	* 运用插值过渡的方式将左右脚的IK值至零
	*/

	
	//Then0

	FootOffset_L_Location = UKismetMathLibrary::VInterpTo
	(
		FootOffset_L_Location,
		FVector(0.0, 0.0, 0.0),
		UGameplayStatics::GetWorldDeltaSeconds(this),
		15.0
	);

	FootOffset_R_Location = UKismetMathLibrary::VInterpTo
	(
		FootOffset_R_Location,
		FVector(0.0, 0.0, 0.0),
		UGameplayStatics::GetWorldDeltaSeconds(this),
		15.0
	);


	//Then1

	FootOffset_L_Rotation = UKismetMathLibrary::RInterpTo
	(
		FootOffset_L_Rotation,
		FRotator(0.0, 0.0, 0.0),
		UGameplayStatics::GetWorldDeltaSeconds(this),
		15.0
	);

	FootOffset_R_Rotation = UKismetMathLibrary::RInterpTo
	(
		FootOffset_R_Rotation,
		FRotator(0.0, 0.0, 0.0),
		UGameplayStatics::GetWorldDeltaSeconds(this),
		15.0
	);


}

FVelocityBlend UALSCharacterAnimInstance::CalculateVelocityBlend()
{
	/*
	* 不旋转向量释义(均是根据世界坐标系来衡量)
	* 根据旋转值来旋转向量，比如向量为（1，0，0），旋转值为（0，90（Yaw），0）
	* 则不旋转向量返回值为（0，-1，0）
	* 旋转向量返回值为（0，1，0）
	* 不旋转向量就是逆时针旋转
	* 旋转向量就是顺时针旋转
	* 
	* 也就是以人物旋转值为基准
	* 将人物速度进行人物旋转值大小的逆旋转
	* 将速度的原坐标系旋转至一个新的坐标系
	*/


	/*
	* 此处将速度各个方向规格化后按下W，LocRelativeVelocityDir.X是1，
	* 按下D，LocRelativeVelocityDir.Y是1，若同时按下WD，
	* 以LocRelativeVelocityDir.X/Y为三角形的直角边，斜边长度为1，所以按下WD的角度为45度，
	* 所以同时按下WD时直角边的分量为二分之根号二（即XY轴的值均为二分之根号二），至于为什么是二分之根号二
	* 那是因为勾股定理，两者的平方相加要等于1，所以是二分之根号二
	* 为了让四向权重，两两相加为1，所以我们要将其化简（此处为理想状态，人物旋转值为0）
	*/
	
	const FVector NormalVelocity = UKismetMathLibrary::Normal(Velocity);
	const FVector LocRelativeVelocityDir = ALSCharacter->GetActorRotation().UnrotateVector(NormalVelocity);

	/*
	* 若按下的是W,D键
	* Sum是将X,Y上的分量进行相加（Z为角色纵向速度，也就是跳跃速度大小，不参与计算）
	* 也就是二分之根号二 + 二分之根号二
	* 则Sum为根号二
	* 
	* 若按下的是单个键
	* 则Sum为1
	* 
	* 若按下WS/AD
	* 则Sum为0
	*/

	const float Sum =
		UKismetMathLibrary::Abs(LocRelativeVelocityDir.X) +
		UKismetMathLibrary::Abs(LocRelativeVelocityDir.Y) +
		UKismetMathLibrary::Abs(LocRelativeVelocityDir.Z);

	/*
	* 若按下的是W,D键
	* 则LocRelativeVelocityDir.X/Y上的分量为二分之根号二
	* 再除以Sum也就是根号二
	* 最终结果为1/2
	* 
	* 若按下的是单个键
	* 则分量为1
	* Sum为1
	* 最终结果为1
	* 
	* 若按下WS/AD
	* 则分量为0
	* Sum为0
	* 最终结果为0
	*/

	const FVector RelativeDirection = LocRelativeVelocityDir / Sum;

	float ClampF = UKismetMathLibrary::FClamp(RelativeDirection.X, 0, 1);
	float ClampB = UKismetMathLibrary::Abs(UKismetMathLibrary::FClamp(RelativeDirection.X, -1, 0));
	float ClampL = UKismetMathLibrary::Abs(UKismetMathLibrary::FClamp(RelativeDirection.Y, -1, 0));
	float ClampR = UKismetMathLibrary::FClamp(RelativeDirection.Y, 0, 1);
	FVelocityBlend ClampBlend;
	ClampBlend.F = ClampF;
	ClampBlend.B = ClampB;
	ClampBlend.L = ClampL;
	ClampBlend.R = ClampR;

	return ClampBlend;
}

float UALSCharacterAnimInstance::CalculateDiagonalAmount()
{
	/*
	* 将对角线上的数值进行曲线映射
	* 该曲线是关于 0-0.5 映射至 0-1 是振荡的
	* 也就是 1为0 ，1.5为1 ，负数也是同理
	* 若按下W/S键，则映射出来的值为0
	* 若按下WD/SD/WA/SA键，则映射出来的值为1
	*/
	const float ReturnValue = DiagonalScaleAmountCurve->GetFloatValue(
		UKismetMathLibrary::Abs(
			VelocityBlend.F + 
			VelocityBlend.B
		)
	);
	
	return ReturnValue;
}

FVector UALSCharacterAnimInstance::CalculateRelativeAccelerationAmount()
{

	/*
	* 该函数是用于计算角色在移动是的加速度方向的相对偏移值
	* 假设我们角色朝向时刻面对世界坐标，当按下S键时
	* 加速度方向位于正后方，角色旋转值位于正前方
	* 规格化之后再进行不旋转向量，角色此时的向量值为（-1，0，0）
	* 同理，我们按下W键时，角色此时的向量值为（1，0，0）
	* 按下A键，角色此时的向量值为（0，1，0）
	* 按下D键，角色此时的向量值为（0，-1，0）
	* 所以，该函数时用于计算角色移动时的加速度朝向，主要使用于角色的LeanAmount
	*/

	//判断加速度方向和速度方向是否是同向（两个向量的夹角是否小于90度）小于90度，数值是大于0，同向，大于90度，数值是小于0，异向
	if (UKismetMathLibrary::Dot_VectorVector(Acceleration, Velocity) > 0)
	{

		FVector ReturnValue = ALSCharacter->GetActorRotation().UnrotateVector(
			UKismetMathLibrary::Vector_ClampSizeMax(
				Acceleration, 
				ALSCharacter->GetCharacterMovement()->GetMaxAcceleration()
			) /
			ALSCharacter->GetCharacterMovement()->GetMaxAcceleration()
		);
		
		return ReturnValue;

	}
	else
	{

		FVector ReturnValue = ALSCharacter->GetActorRotation().UnrotateVector(
			UKismetMathLibrary::Vector_ClampSizeMax(
				Acceleration,
				ALSCharacter->GetCharacterMovement()->GetMaxBrakingDeceleration()
			) /
			ALSCharacter->GetCharacterMovement()->GetMaxBrakingDeceleration()
		);

		return ReturnValue;

	}

}

float UALSCharacterAnimInstance::CalculateWalkRunBlend()
{
	
	/*
	* 通过Gait修改行走和奔跑的比例
	* 根据混合率的不同判断角色处于行走还是奔跑状态
	* 行走步距的混合为0
	* 奔跑冲刺的混合为1
	*/

	//判断角色步态是否是行走
	if (Gait == EALSGait::EG_Walking)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

float UALSCharacterAnimInstance::CalculateStrideBlend()
{
	/*
	* 步距是于动画中使用
	* 根据不同步距的大小判断角色处于不同的动画中
	* 0-1对应着角色的Idle-Walk/Run（其实最低的是0.2，不过混合空间最低是0）
	* 行走步距混合曲线映射根据Speed映射 0-150 -> 0.2-1
	* 奔跑步距混合曲线映射根据Speed映射 0-350 -> 0.2-1
	* 蹲伏行走步距混合曲线映射根据Speed映射 0-150 -> 0.2-1
	*/
	
	const float LocWalkToRunStride = UKismetMathLibrary::Lerp(
		StrideBlendNWalk->GetFloatValue(Speed),
		StrideBlendNRun->GetFloatValue(Speed),
		GetAnimCurveClamped(FName("Weight_Gait"),-1,0,1)
	);

	const float LocStandToCrouchStride = UKismetMathLibrary::Lerp(
		LocWalkToRunStride,
		StrideBlendCWalk->GetFloatValue(Speed),
		GetCurveValue(FName("Basepose_CLF"))
	);

	return LocStandToCrouchStride;
}

float UALSCharacterAnimInstance::CalculateStandingPlayRate()
{
	/*
	* 计算站立状态下的播放率
	* 通过使用Speed除以角色的动画速度
	* 然后再通过宏GetAnimCurveClamped来判断角色是处于哪个状态
	* 然后再把Speed除以角色的动画速度得到的值（0-1）
	* 除以步距（0.2-1）
	* 最后再除以世界缩放
	* 最终结果就是角色的各个状态下的动画播放率
	*/
	const float WalkToRunPlayRate = UKismetMathLibrary::Lerp(
		Speed / AnimatedWalkSpeed,
		Speed / AnimatedRunSpeed,
		GetAnimCurveClamped(FName("Weight_Gait"), -1, 0, 1)
	);

	const float WalkToRunToSprintPlayRate = UKismetMathLibrary::Lerp(
		WalkToRunPlayRate,
		Speed / AnimatedSprintSpeed,
		GetAnimCurveClamped(FName("Weight_Gait"), -2, 0, 1)
	);

	return UKismetMathLibrary::FClamp(
		(WalkToRunToSprintPlayRate / StrideBlend) / GetOwningComponent()->GetComponentScale().Z,
		0,
		3
	);
}

float UALSCharacterAnimInstance::CalculateCrouchingPlayRate()
{
	
	/*
	* 计算蹲伏状态下的播放率
	* 通过使用Speed除以角色的动画速度
	* 然后再把Speed除以角色的动画速度得到的值（0-1）
	* 除以步距（0.2  -  1 ）
	* 最后再除以世界缩放
	* 最终结果就是角色的各个状态下的动画播放率
	*/

	return UKismetMathLibrary::FClamp(
		((Speed / AnimatedCrouchSpeed) / StrideBlend) / GetOwningComponent()->GetComponentScale().Z,
		0,
		2
	);

}

EALSMovementDirection UALSCharacterAnimInstance::CalculateMovementDirection()
{
	//判断角色步态
	if (Gait == EALSGait::EG_Walking ||
		Gait == EALSGait::EG_Running)
	{
		//判断角色旋转模式
		if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
		{
			return EALSMovementDirection::EMD_ForWard;
		}
		else
		{
			/*
			* 将角色速度方向（也就是运动输入方向）与角色锚点世界坐标向量进行从X旋转向量后得到的旋转值（WD时旋转值为45，W/S时旋转值为0/180）
			* 和瞄准方向（也就是控制器正对的方向）进行一个差量计算
			* 最后得到角色旋转的角度
			* 
			* 70 FR方向的阈值
			* -70 FL方向的阈值
			* 110 BR方向的阈值
			* -110 BL方向的阈值
			* 5 偏移量
			*/
			return CalculateQuadrant(
				MovementDirection,
				70,
				-70,
				110,
				-110,
				5,
				UKismetMathLibrary::NormalizedDeltaRotator(
					Velocity.ToOrientationRotator(),
					AimingRotation
				).Yaw
			);
		}
	}
	else
	{
		return EALSMovementDirection::EMD_ForWard;
	}
}

EALSMovementDirection UALSCharacterAnimInstance::CalculateQuadrant(EALSMovementDirection Current, float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer, float Angle)
{

	/*
	* 不动鼠标，只动键盘时
	* 只有同时移动鼠标键盘时，才有可能获得其他数值
	* F = 0
	* FL = -45 
	* FR = 45
	* L = -90 
	* R = 90
	* BL = -135
	* BR = 135
	* B = 180/-180
	*/

	/*
	* 此处判断角色是否不为前向或者后向，只要满足MovementDirection为前向或者后向则布尔值为真（粗判断）
	* 然后在对Angle进行角度阈值判断是前向还是后向（细判断）
	* 假设此时按下WD键，Angle为45度
	* 同时FL和FR为-70和70
	* 此时Angle满足-75 -- 75 的范围（此范围为前向）
	* 则此处布尔值返回真，同时改变MovementDirection为前向
	*/
	if (AngleInRange
	(
		Angle,
		FL_Threshold,
		FR_Threshold,
		Buffer,
		Current != EALSMovementDirection::EMD_ForWard || Current != EALSMovementDirection::EMD_BackWard
	)
		)
	{
		return EALSMovementDirection::EMD_ForWard;
	}
	/*
	* 此处判断角色是否不为左向或者右向，只要满足MovementDirection为左向或者右向则布尔值为真（粗判断）
	* 然后在对Angle进行角度阈值判断是左向还是右向（细判断）
	* 假设此时按下D键，Angle为90度
	* 同时FR和BR为70和110
	* 此时Angle满足65 -- 115 的范围（此范围为右向）
	* 则此处布尔值返回真，同时改变MovementDirection为右向
	*/
	else if (
		AngleInRange
		(
			Angle,
			FR_Threshold,
			BR_Threshold,
			Buffer,
			Current != EALSMovementDirection::EMD_Right || Current != EALSMovementDirection::EMD_Left
		)
			)
	{
		return EALSMovementDirection::EMD_Right;
	}
	/*
	* 此处判断角色是否不为左向或者右向，只要满足MovementDirection为左向或者右向则布尔值为真（粗判断）
	* 然后在对Angle进行角度阈值判断是左向还是右向（细判断）
	* 假设此时按下A键，Angle为-90度
	* 同时FL和BL为-70和-110
	* 此时Angle满足-65 -- -115 的范围（此范围为左向）
	* 则此处布尔值返回真，同时改变MovementDirection为左向
	*/
	else if (AngleInRange
	(
		Angle,
		BL_Threshold,
		FL_Threshold,
		Buffer,
		Current != EALSMovementDirection::EMD_Right || Current != EALSMovementDirection::EMD_Left
	)
			)
	{
		return EALSMovementDirection::EMD_Left;
	}
	//若以上条件（前后左右布尔值，角度阈值范围）均不满足，则角色方向为后向
	else
	{
		return EALSMovementDirection::EMD_BackWard;
	}
}

bool UALSCharacterAnimInstance::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	/*
	* IncreaseBuffer基本为真, 所以会走else
	* 布尔值为真值时为小范围
	* 布尔值为假值时为大范围
	* 基本上只会在大范围的数值中进行布尔判断
	*/
	if (!IncreaseBuffer)
	{
		return UKismetMathLibrary::InRange_FloatFloat(
			Angle,
			MinAngle + Buffer,
			MaxAngle - Buffer
		);
	}
	else
	{
		return UKismetMathLibrary::InRange_FloatFloat(
			Angle,
			MinAngle - Buffer,
			MaxAngle + Buffer
		);
	}
}

FVelocityBlend UALSCharacterAnimInstance::InterpVelocityBlend(FVelocityBlend Current, FVelocityBlend Target, float InterpSpeed, float DeltaTime)
{
	//将速度方向混合的数值进行插值过渡，使其变化的更加自然
	FVelocityBlend ReturnValue;

	ReturnValue.F = UKismetMathLibrary::FInterpTo(
		Current.F,
		Target.F,
		DeltaTime,
		InterpSpeed
	);
	ReturnValue.B = UKismetMathLibrary::FInterpTo(
		Current.B,
		Target.B,
		DeltaTime,
		InterpSpeed
	);
	ReturnValue.L = UKismetMathLibrary::FInterpTo(
		Current.L,
		Target.L,
		DeltaTime,
		InterpSpeed
	);
	ReturnValue.R = UKismetMathLibrary::FInterpTo(
		Current.R,
		Target.R,
		DeltaTime,
		InterpSpeed
	);
	return ReturnValue;
}

FLeanAmount UALSCharacterAnimInstance::InterpLeanAmount(FLeanAmount Current, FLeanAmount Target, float InterpSpeed, float DeltaTime)
{
	//将人物倾斜的数值进行插值过渡，使其变化的更加自然
	FLeanAmount ReturnValue;

	ReturnValue.FB = UKismetMathLibrary::FInterpTo(
		Current.FB,
		Target.FB,
		DeltaTime,
		InterpSpeed
	);
	ReturnValue.LR = UKismetMathLibrary::FInterpTo(
		Current.LR,
		Target.LR,
		DeltaTime,
		InterpSpeed
	);
	return ReturnValue;
}

float UALSCharacterAnimInstance::GetAnimCurveClamped(FName Name, float Bias, float ClampMin, float ClampMax)
{
	//获取动画曲线（动画序列中的）对应的数值，然后进行限制映射，最终返回
	const float ReturnValue = UKismetMathLibrary::FClamp(
		GetCurveValue(Name) + Bias,
		ClampMin,
		ClampMax
	);

	return ReturnValue;

}

float UALSCharacterAnimInstance::GetAnimCurveCompact(FName Name)
{
	return GetCurveValue(Name);
}
