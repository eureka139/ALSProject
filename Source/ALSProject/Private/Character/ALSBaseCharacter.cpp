#include "Character/ALSBaseCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/ALSCharacterAnimInstance.h"
#include "Curves/CurveVector.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "Components/TimelineComponent.h"

/*
* 接口函数重载
*/

//获取第三人称检测信息
void AALSBaseCharacter::GetTPTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel)
{
	TraceOrigin = GetActorLocation();
	TraceRadius = 10.f;
	TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
}
//获取第三人称锚点位置
void AALSBaseCharacter::GetTPPivotTarget(FTransform& ReturnValue)
{
	ReturnValue = GetActorTransform();
}
//获取第一人称摄像机位置
void AALSBaseCharacter::GetFPCameraTarget(FVector& ReturnValue)
{
	ReturnValue = GetMesh()->GetSocketLocation(FName("FP_Camera"));
}
//获取摄像机参数值
void AALSBaseCharacter::GetCameraParameters(float& TPFOV, float& FPFOV, bool& RightShoulder)
{
	TPFOV = ThirdPersonFOV;
	FPFOV = FirstPersonFOV;
	RightShoulder = bRightShoulder;
}

void AALSBaseCharacter::SetMovementState(EALSMovementState NewMovementState, bool bForce)
{
	//判断当前运动状态枚举是否与新运动状态枚举一致,若不一致则返回新运动状态枚举
	if (MovementState != NewMovementState || bForce)
	{
		//将当前的运动状态枚举赋给上一时刻
		PrevMovementState = MovementState;
		//将新运动状态枚举赋给当前
		MovementState = NewMovementState;
	}
	//判断当前运动状态是否处于空中
	if (MovementState == EALSMovementState::EMS_InAir)
	{
		//判断角色当前蒙太奇操作是否为空
		if (MovementAction == EALSMovementAction::EMA_None)
		{
			//将ActorRotation赋值给角色位于空中时的Rotation
			InAirRotation = GetActorRotation();
			//若角色当前处于蹲伏状态时则取消蹲伏
			if (Stance == EALSStance::ES_Crouching)
			{
				UnCrouch();
			}
		}
		//判断角色当前蒙太奇操作是否为翻滚蒙太奇
		else if (MovementAction == EALSMovementAction::EMA_Rolling)
		{
			RagdollStart();
		}
	}
	//判断角色当前状态是否为物理模拟状态
	else if (MovementState == EALSMovementState::EMS_Ragdoll)
	{
		//判断上一时刻状态是否为攀爬状态
		if (PrevMovementState == EALSMovementState::EMS_Mantling)
		{
			//若为攀爬则停止攀爬时间轴
			MantleTimeline->Stop();
		}
	}
}

void AALSBaseCharacter::SetMovementAction(EALSMovementAction NewMovementAction, bool bForce)
{

	EALSMovementAction PrevMovementAction;

	//判断当前角色蒙太奇动画枚举是否与新蒙太奇动画枚举一致,若不一致则返回新蒙太奇动画枚举
	if (MovementAction != NewMovementAction || bForce)
	{
		//将当前的蒙太奇动画枚举赋给上一时刻
		PrevMovementAction = MovementAction;
		//将新蒙太奇动画枚举赋给当前
		MovementAction = NewMovementAction;
	}

	//Then0

	if (MovementAction == EALSMovementAction::EMA_Rolling)
	{
		Crouch();
	}

	//Then1

	if (PrevMovementAction == EALSMovementAction::EMA_Rolling)
	{
		if (DesiredStance == EALSStance::ES_Standing)
		{
			UnCrouch();
		}
		else
		{
			Crouch();
		}
	}
	
}

void AALSBaseCharacter::SetRotationMode(EALSRotationMode NewRotationMode, bool bForce)
{
	//判断当前旋转模式是否与新旋转模式一致,若不一致则返回新旋转模式
	if (RotationMode != NewRotationMode || bForce)
	{
		EALSRotationMode PrevRotationMode;
		//将当前的旋转模式赋给上一时刻
		PrevRotationMode = RotationMode;
		//将新旋转模式赋给当前
		RotationMode = NewRotationMode;
	}

	if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
	{
		if (ViewMode == EALSViewMode::EVM_FirstPerson)
		{
			IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
			ALSCharacterInterface->SetViewMode(EALSViewMode::EVM_ThirdPerson);
		}
	}
}

void AALSBaseCharacter::SetGait(EALSGait NewGait, bool bForce)
{
	//判断当前步态是否与新步态一致,若不一致则返回新步态
	if (Gait != NewGait || bForce)
	{
		EALSGait PrevGait;
		//将当前的步态赋给上一时刻
		PrevGait = Gait;
		//将新步态赋给当前
		Gait = NewGait;
	}
}

void AALSBaseCharacter::SetViewMode(EALSViewMode NewViewMode, bool bForce)
{
	//判断当前视角模式是否与新视角模式一致,若不一致则返回新视角模式
	if (ViewMode != NewViewMode || bForce)
	{
		EALSViewMode PrevViewMode;
		//将当前的视角模式赋给上一时刻
		PrevViewMode = ViewMode;
		//将新视角模式赋给当前
		ViewMode = NewViewMode;
	}

	if (ViewMode == EALSViewMode::EVM_ThirdPerson)
	{
		if (RotationMode == EALSRotationMode::ERM_VelocityDirection ||
			RotationMode == EALSRotationMode::ERM_LookingDirection)
		{
			IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
			ALSCharacterInterface->SetRotationMode(DesiredRotationMode);
		}
	}
	else
	{

		if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
		{
			IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
			ALSCharacterInterface->SetRotationMode(EALSRotationMode::ERM_LookingDirection);
		}

	}
}

void AALSBaseCharacter::SetOverlayState(EALSOverlayState NewOverlayState, bool bForce)
{
	//判断当前叠加状态是否与新叠加状态一致,若不一致则返回新叠加状态
	if (OverlayState != NewOverlayState || bForce)
	{
		EALSOverlayState PrevOverlayState;
		//将当前的叠加状态赋给上一时刻
		PrevOverlayState = OverlayState;
		//将新叠加状态赋给当前
		OverlayState = NewOverlayState;
	}
}

void AALSBaseCharacter::SetStance(EALSStance NewStance, bool bForce)
{

	//判断当前叠加状态是否与新叠加状态一致,若不一致则返回新叠加状态
	if (Stance != NewStance || bForce)
	{
		EALSStance PrevStance;
		//将当前的叠加状态赋给上一时刻
		PrevStance = Stance;
		//将新叠加状态赋给当前
		Stance = NewStance;
	}

}

void AALSBaseCharacter::GetCurrentState(EMovementMode& IPawnMovementMode, EALSMovementState& IMovementState, EALSMovementState& IPrevMovementState, EALSMovementAction& IMovementAction, EALSRotationMode& IRotationMode, EALSGait& IActualGait, EALSStance& IActualStance, EALSViewMode& IViewMode, EALSOverlayState& IOverlayState)
{
	/*
	* 将人物的枚举信息赋给接口函数中
	*/
	IPawnMovementMode = GetCharacterMovement()->MovementMode;
	IMovementState = MovementState;
	IPrevMovementState = PrevMovementState;
	IMovementAction = MovementAction;
	IRotationMode = RotationMode;
	IActualGait = Gait;
	IActualStance = Stance;
	IViewMode = ViewMode;
	IOverlayState = OverlayState;
}

void AALSBaseCharacter::GetEssentialValues(FVector& IVelocity, FVector& IAcceleration, FVector& IMovementInput, bool& IbIsMoving, bool& IbHasMovingInput, float& ISpeed, float& IMovementInputAmount, float& IAimYawRate, FRotator& IAimingRotation)
{
	/*
	* 将人物中的主要信息赋给接口函数中
	*/
	IVelocity = GetVelocity();
	IAcceleration = Acceleration;
	IMovementInput = GetCharacterMovement()->GetCurrentAcceleration();
	IbIsMoving = bIsMoving;
	IbHasMovingInput = bHasMovementInput;
	ISpeed = Speed;
	IMovementInputAmount = MovementInputAmount;
	IAimYawRate = AimYawRate;
	IAimingRotation = GetControlRotation();
}



AALSBaseCharacter::AALSBaseCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	//给角色添加Actor标签
	Tags.Add(FName("ALS_Character")); //角色标签

}

void AALSBaseCharacter::OnBeginPlay()
{
	//Then0

	//确保人物是有效的,若无效则Tick函数是无法执行的
	GetMesh()->AddTickPrerequisiteActor(this);

	//Then1

	//判断角色动画实例是否有效
	if (IsValid(GetMesh()->GetAnimInstance()))
	{
		//然后将人物和动画实例进行通讯
		MainAnimInstance = Cast<UALSCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	}

	//Then2

	//使用数据表中的数据给结构体进行初始化
	SetMovementModel();

	//Then3

	/*
	* 初始化枚举信息，将期望的枚举值赋值给当前的枚举中
	*/

	SetGait(DesiredGait,true);

	SetRotationMode(DesiredRotationMode,true);

	SetViewMode(ViewMode,true);

	SetOverlayState(OverlayState,true);

	//判断期望的步态是否等于站立
	if (DesiredStance == EALSStance::ES_Standing)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}

	//Then4

	/*
	* 初始化人物旋转值，速度方向的旋转值，加速度方向的旋转值
	*/

	LastVelocityRotation = GetActorRotation();
	LastAccelerationRotation = GetActorRotation();
	TargetRotation = GetActorRotation();
}

void AALSBaseCharacter::SetMovementModel()
{
	/*
	* 创建一个数据表，然后在编辑器中赋值，然后在cpp中初始化该数据表，将编辑器中的信息赋给该数据表中
	* 然后从数据表中获取我们所需的那一行（Normal）
	* 将其赋值给结构体MovementData
	*/
	const FString ContexString = GetFullName();
	FMovementSettingsState* OutRow =
		MovementModel.DataTable->FindRow<FMovementSettingsState>(MovementModel.RowName, ContexString);
	check(OutRow);
	MovementData = *OutRow;
}

void AALSBaseCharacter::UpdateCharacterMovement()
{
	//Then0

	//获取允许的步态
	EALSGait AllowGait = GetAllowedGait();


	//Then1

	/*
	* 将允许的步态提升到实际的步态，判断实际步态是否与Gait不相同，若不相同则将实际步态传给Gait
	*/

	EALSGait ActualGait = GetActualGait(AllowGait);

	if (ActualGait != Gait)
	{
		IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
		ALSCharacterInterface->SetGait(ActualGait);
	}

	//Then2

	UpdateDynamicMovementSettings(AllowGait);

}

EALSGait AALSBaseCharacter::GetAllowedGait()
{
	//判断角色当前是否是站立状态
	if (Stance == EALSStance::ES_Standing)
	{
		//判断角色旋转模式是否是非瞄准状态
		if (RotationMode == EALSRotationMode::ERM_VelocityDirection||
			RotationMode == EALSRotationMode::ERM_LookingDirection 
			)
		{
			//若期望的步态为Walking则返回允许的步态Walking
			if (DesiredGait == EALSGait::EG_Walking)
			{
				return EALSGait::EG_Walking;
			}
			//若期望的步态为Running则返回允许的步态Running
			else if (DesiredGait == EALSGait::EG_Running)
			{
				return EALSGait::EG_Running;
			}
			//判断角色当前是否可以进行冲刺，若可以则返回允许的步态Sprinting
			else if (CanSprint())
			{
				return EALSGait::EG_Sprinting;
			}
			//若不能冲刺则返回允许的步态Running
			else
			{
				return EALSGait::EG_Running;
			}
		}
		//若角色当前处于瞄准状态
		else
		{
			//判断期望的步态是否为Walking
			if (DesiredGait == EALSGait::EG_Walking)
			{
				return EALSGait::EG_Walking;
			}
			//若不为Walking，则均返回Running
			else
			{
				return EALSGait::EG_Running;
			}
		}
	}
	//若角色处于蹲伏状态
	else
	{
		//判断期望的步态是否为Walking
		if (DesiredGait == EALSGait::EG_Walking)
		{
			return EALSGait::EG_Walking;
		}
		//若不为Walking，则均返回Running
		else
		{
			return EALSGait::EG_Running;
		}
	}
	
}

bool AALSBaseCharacter::CanSprint()
{
	//判断是否有运动输入
	if (bHasMovementInput)
	{
		//判断角色当前旋转模式是否是速度方向
		if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
		{
			//若处于速度方向，再判断运动输入是否是大于0.9
			if (MovementInputAmount > 0.9f)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		//判断角色当前旋转模式是否是控制器方向
		else if (RotationMode == EALSRotationMode::ERM_LookingDirection)
		{
			/*
			* 从X向量旋转释义(ToOrientationRotator)
			* 若有速度大小，则该旋转体记录的是最后一处速度大小的旋转值大小
			* 也就是世界坐标下速度的朝向，与按下的轴值有关，按下wd则yaw为45，按下d为90，按下w为0
			* 计算世界坐标中x轴方向旋转至速度方向后与原x轴方向之间的旋转体大小
			*/

			/*
			 * 假设我们开始游戏时，未挪动控制器（鼠标）此时控制器的旋转朝向为（0，0，0）
			 * 然后我们按下D的同时，返回给我们的向量旋转为90（Yaw轴）
			 * 然后和控制选择做差量计算，得出差值为90（Yaw轴）返回差量计算的小角度
			 * 按下WD就是45，以此类推就是得出结果
			 */
			
			FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(
				GetCharacterMovement()->GetCurrentAcceleration().ToOrientationRotator(),
				GetControlRotation()
			);
			/*
			* 判断运动输入是否大于0.9并且角色加速度方向的旋转值和控制器方向的旋转值之间Yaw的差值是否大于50
			* 也就是说，角色只能F（0）,FL（45）,FR（45）方向才能进行冲刺，超越50度的方向都不能冲刺
			*/
			if (MovementInputAmount > 0.9f &&
				UKismetMathLibrary::Abs(DeltaRotation.Yaw) < 50.0f
				)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		//若旋转模式是瞄准方向，则不可以冲刺
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}

EALSGait AALSBaseCharacter::GetActualGait(EALSGait AllowedGait)
{

	/*
	* 通过速度的判断，将允许的步态改变为实际的步态，之后再把实际的步态是要赋给Gait
	* 然后将信息传递给动画实例
	*/

	//将编辑器中初始化结构体的数值赋进去

	const float LocalWalkSpeed = CurrentMovementSettings.WalkSpeed; //165
	const float LocalRunSpeed = CurrentMovementSettings.RunSpeed; //350
	const float LocalSprintSpeed = CurrentMovementSettings.SprintSpeed; //600

	//若角色Speed大于350+10，也就是处于Run至Sprint之间
	if (Speed >= LocalRunSpeed + 10)
	{
		//在对其允许的步态进行判断，若允许的步态为Walk或Run，则将两者归类为Run
		if (AllowedGait == EALSGait::EG_Walking ||
			AllowedGait == EALSGait::EG_Running
			)
		{
			return EALSGait::EG_Running;
		}
		//若允许的步态为Sprint，则保持其步态，不做改变
		else
		{
			return EALSGait::EG_Sprinting;
		}
	}
	//若角色Speed小于350+10大于165+10，也就是处于Walk至Run之间的状态，将其归类为Run
	else if (Speed >= LocalWalkSpeed + 10)
	{
		return EALSGait::EG_Running;
	}
	//若角色Speed小于165+10，也就是处于Idel至Walk之间的状态，将其归类为Walk
	else
	{
		return EALSGait::EG_Walking;
	}
}

void AALSBaseCharacter::UpdateDynamicMovementSettings(EALSGait AllowedGait)
{
	//将编辑器中数据表内的信息赋值给CurrentMovementSettings
	CurrentMovementSettings = GetTargetMovementSettings();

	//Then0
	
	//判断允许的步态是否是Walk，是则将结构体中的Speed赋给MaxWalkSpeed和MaxWalkSpeedCrouched
	if (AllowedGait == EALSGait::EG_Walking)
	{
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;
	}
	//判断允许的步态是否是Run，是则将结构体中的Speed赋给MaxWalkSpeed和MaxWalkSpeedCrouched
	else if (AllowedGait == EALSGait::EG_Running)
	{
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.RunSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;
	}
	//判断允许的步态是否是Sprint，是则将结构体中的Speed赋给MaxWalkSpeed和MaxWalkSpeedCrouched
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.SprintSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;
	}

	//Then1

	//获取速度映射（0-3）对应的曲线值
	FVector CurveVector = CurrentMovementSettings.MovementCurves->GetVectorValue(GetMappedSpeed());

	//将曲线映射后的数值赋给角色移动当中（最大加速度，制动降速行走，地面摩擦力）
	GetCharacterMovement()->MaxAcceleration = CurveVector.X;
	GetCharacterMovement()->BrakingDecelerationWalking = CurveVector.Y;
	GetCharacterMovement()->GroundFriction = CurveVector.Z;
	

}

FMovementSettings AALSBaseCharacter::GetTargetMovementSettings()
{

	/*
	* 实时更新从编辑器中数据表传递来的数值信息
	* 将其同步到CurrentMovementSettings
	*/


	//判断当前角色旋转模式是否是速度方向
	if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
	{
		//判断当前角色姿态是否是站立状态
		if (Stance == EALSStance::ES_Standing)
		{
			return MovementData.VelocityDirection.Standing;
		}
		//判断当前角色姿态是否是蹲伏状态
		else
		{
			return MovementData.VelocityDirection.Crouching;
		}
	}
	//判断当前角色旋转模式是否是视角方向
	else if (RotationMode == EALSRotationMode::ERM_LookingDirection)
	{
		//判断当前角色姿态是否是站立状态
		if (Stance == EALSStance::ES_Standing)
		{
			return MovementData.LookingDirection.Standing;
		}
		//判断当前角色姿态是否是蹲伏状态
		else
		{
			return MovementData.LookingDirection.Crouching;
		}
	}
	//判断当前角色旋转模式是否是瞄准方向
	else
	{
		//判断当前角色姿态是否是站立状态
		if (Stance == EALSStance::ES_Standing)
		{
			return MovementData.Aiming.Standing;
		}
		//判断当前角色姿态是否是蹲伏状态
		else
		{
			return MovementData.Aiming.Crouching;
		}
	}
}

float AALSBaseCharacter::GetMappedSpeed()
{

	/*
	* 将0到600的数值映射到0到3
	*/

	//提取编辑器中数据表里结构体的数值信息
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed; //165
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed; //350
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed; //600

	/*
	* Idle为0
	* Walk为1
	* Run为2
	* Sprint为3
	*/

	//将0-WalkSpeed映射至0-1
	float MapIdleToWalk = UKismetMathLibrary::MapRangeClamped(
		Speed,
		0.0f,
		LocWalkSpeed,
		0.0f,
		1.0f
	);
	//将WalkSpeed-RunSpeed映射至1-2
	float MapWalkToRun = UKismetMathLibrary::MapRangeClamped(
		Speed,
		LocWalkSpeed,
		LocRunSpeed,
		1.0f,
		2.0f
	);
	//将RunSpeed-SprintSpeed映射至2-3
	float MapRunToSprint = UKismetMathLibrary::MapRangeClamped(
		Speed,
		LocRunSpeed,
		LocSprintSpeed,
		2.0f,
		3.0f
	);
	//速度大于Walk
	if (Speed > LocWalkSpeed)
	{
		//速度大于Run
		if (Speed > LocRunSpeed)
		{
			//返回2-3
			return MapRunToSprint;
		}
		//速度小于Run
		else
		{
			//返回1-2
			return MapWalkToRun;
		}
	}
	//速度小于Walk
	else
	{
		//速度大于Run
		if (Speed > LocRunSpeed)
		{
			//返回2-3
			return MapRunToSprint;
		}
		//速度小于Run
		else
		{
			//返回0-1
			return MapIdleToWalk;
		}
	}
}

void AALSBaseCharacter::UpdateGroundedRotation()
{
	//判断角色当前有无蒙太奇动画播放
	if (MovementAction == EALSMovementAction::EMA_None)
	{
		//判断角色当前是否正在移动或者正在进行某些根运动
		if (CanUpdateMovingRotation())
		{
			//判断角色当前的旋转模式
			if (RotationMode == EALSRotationMode::ERM_VelocityDirection)
			{
				SmoothCharacterRotation(
					FRotator(0.0, LastVelocityRotation.Yaw, 0.0),
					800.0,
					CalculateGroundedRotationRate()
				);
			}
			else if (RotationMode == EALSRotationMode::ERM_LookingDirection)
			{
				//判断角色当前的步态
				if (Gait == EALSGait::EG_Walking ||
					Gait == EALSGait::EG_Running)
				{
					SmoothCharacterRotation(
						FRotator(0, GetControlRotation().Yaw + GetAnimCurvesValue(FName("YawOffset")), 0),
						500,
						CalculateGroundedRotationRate()
					);
				}
				else
				{
					SmoothCharacterRotation(
						FRotator(0, LastVelocityRotation.Yaw, 0),
						500,
						CalculateGroundedRotationRate()
					);
				}
			}
			else if(RotationMode == EALSRotationMode::ERM_Aiming)
			{
				SmoothCharacterRotation(
					FRotator(0.0, GetControlRotation().Yaw, 0.0),
					1000.0,
					20.0
				);
			}
		}
		//不在运动则可以执行原地转向等操作
		else
		{
			//判断视口模式是否是第三人称
			if (ViewMode == EALSViewMode::EVM_ThirdPerson)
			{
				//判断旋转模式是否是速度方向或视口方向
				if (RotationMode == EALSRotationMode::ERM_VelocityDirection ||
					RotationMode == EALSRotationMode::ERM_LookingDirection)
				{
					//判断曲线值的绝对值是否大于0
					if (
						UKismetMathLibrary::Abs(GetAnimCurvesValue(FName("RotationAmount"))) > 0.001
						)
					{
						/*
						* 将缩放后的曲线值乘以（DeltaTime / 三十分之一）
						* 然后将数值赋值给添加Actor世界旋转中去
						* 也就是实时更新角色的世界坐标下的旋转
						*/
						AddActorWorldRotation(
							FRotator(
								0,
								GetAnimCurvesValue(FName("RotationAmount")) * (UGameplayStatics::GetWorldDeltaSeconds(this) / (1.0 / 30.0)),
								0
							)
						);
						TargetRotation = GetActorRotation();
					}

				}
				else
				{
					LimitRotation(-100, 100, 20);
					//判断曲线值的绝对值是否大于0
					if (
						UKismetMathLibrary::Abs(GetAnimCurvesValue(FName("RotationAmount")) > 0.001)
						)
					{
						/*
						* 将缩放后的曲线值乘以（DeltaTime / 三十分之一）
						* 然后将数值赋值给添加Actor世界旋转中去
						* 也就是实时更新角色的世界坐标下的旋转
						*/
						AddActorWorldRotation(
							FRotator(
								0,
								GetAnimCurvesValue(FName("RotationAmount")) * (UGameplayStatics::GetWorldDeltaSeconds(this) / (1.0 / 30.0)),
								0
							)
						);
						TargetRotation = GetActorRotation();
					}
				}
			}
			else
			{
				LimitRotation(-100, 100, 20);
				//判断曲线值的绝对值是否大于0
				if (
					UKismetMathLibrary::Abs(GetAnimCurvesValue(FName("RotationAmount")) > 0.001)
					)
				{
					/*
					* 将缩放后的曲线值乘以（DeltaTime / 三十分之一）
					* 然后将数值赋值给添加Actor世界旋转中去
					* 也就是实时更新角色的世界坐标下的旋转
					*/
					AddActorWorldRotation(
						FRotator(
							0,
							GetAnimCurvesValue(FName("RotationAmount")) * (UGameplayStatics::GetWorldDeltaSeconds(this) / (1.0 / 30.0)),
							0
						)
					);
					TargetRotation = GetActorRotation();
				}
			}
			
		}
	}
	else if (MovementAction == EALSMovementAction::EMA_Rolling)
	{

		if (bHasMovementInput)
		{
			SmoothCharacterRotation(
				FRotator(0.0, LastAccelerationRotation.Yaw, 0.0),
				0.0,
				2.0
			);
		}

	}

}

bool AALSBaseCharacter::CanUpdateMovingRotation()
{
	//判断是否正在运动并且有移动输入，或者Speed大于150，并且角色当前不正在进行任何根运动
	return ((bIsMoving && bHasMovementInput) || Speed > 150) && !HasAnyRootMotion();
}

void AALSBaseCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed)
{
	//每一帧利用插值过渡的方式来更新TargetRotation（主要是Yaw值）
	TargetRotation = UKismetMathLibrary::RInterpTo_Constant(
		TargetRotation,
		Target,
		UGameplayStatics::GetWorldDeltaSeconds(this),
		TargetInterpSpeed
	);
	//再将人物当前旋转值插值到TargetRotation，然后再返回给人物，重新设置人物旋转
	SetActorRotation(
		UKismetMathLibrary::RInterpTo(
			GetActorRotation(),
			TargetRotation,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			ActorInterpSpeed
		)
	);
}

float AALSBaseCharacter::CalculateGroundedRotationRate()
{
	//根据映射的速度（0-3）来获取曲线中的数值（0.5，2，4，8，20）再乘以AimYawRate的映射值,最后用于Actor的插值速度当中
	return CurrentMovementSettings.RotationRateCurves->GetFloatValue(GetMappedSpeed()) *
		UKismetMathLibrary::MapRangeClamped(
			AimYawRate,
			0,
			300,
			1,
			3
		);
}

void AALSBaseCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	//判断控制器旋转值和Actor旋转值的差值是否处于（-100 - 100）范围内，若不处于才可以执行下一步
	if (
		!UKismetMathLibrary::InRange_FloatFloat(
			UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation()).Yaw,
			AimYawMin,
			AimYawMax)
		)
	{
		/*
		* 若差值位于范围外
		* 将控制器旋转Yaw值分别加上Max（100），Min（-100）
		* 并判断差量是否大于0
		* 若大于0则将控制器旋转Yaw值加上Max（100）赋值给SmoothCharacterRotation
		* 若小于0则将控制器旋转Yaw值加上Min（-100）赋值给SmoothCharacterRotation
		*/
		SmoothCharacterRotation(
			FRotator(0, UKismetMathLibrary::SelectFloat(
				GetControlRotation().Yaw + AimYawMin,
				GetControlRotation().Yaw + AimYawMax,
				UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation()).Yaw > 0
			), 0),
			0.0,
			InterpSpeed
		);
	}
}

void AALSBaseCharacter::UpdateInAirRotation()
{
	//判断角色当前旋转模式是否为速度方向或者视口方向
	if (RotationMode == EALSRotationMode::ERM_VelocityDirection ||
		RotationMode == EALSRotationMode::ERM_LookingDirection)
	{
		//此处的旋转值为人物在空中的朝向方向
		SmoothCharacterRotation(FRotator(0.0, InAirRotation.Yaw, 0.0), 0.0, 5.0);
	}
	else
	{
		//若为瞄准方向则将控制器旋转Yaw值作为角色当前的旋转值
		SmoothCharacterRotation(FRotator(0.0, GetControlRotation().Yaw, 0.0), 0.0, 15.0);
		InAirRotation = GetActorRotation();
	}

}

bool AALSBaseCharacter::MantleCheck(FMantleTraceSettings TraceSettings, ETraceTypeQuery DebugType)
{

	//Then0

	/*
	* TraceDirection为Actor向前向量也就是人物的检测方向
	* Start往后乘以-30是因为防止检测薄墙时检测不到，所以以人物锚点为基准往后移动30，这里是第一帧时的胶囊体检测
	* End是以-30为基准再往前延伸70再进行第二次检测，以两个检测胶囊体间的距离为检测距离
	* 此处检测是以start为基准生成一个胶囊体射线，然后在end处再生成一个胶囊体射线，两个胶囊体射线之间的的距离为检测的全路径
	* Radius是胶囊体半径为30cm
	* HalfHeight （ （150 - 50） /2 ） +1 结果为51，这是检测胶囊体的半高
	*/
	FHitResult CapsuleHitResult;
	TArray<AActor*>CapsuleActorsToIgnore;
	const FVector& TraceDirection = GetActorForwardVector();
	FVector CapsuleStart = GetCalpsuleBaseLocation(2.0) + (TraceDirection * -30.0);
	CapsuleStart.Z += ((TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2.0);
	FVector CapsuleEnd = CapsuleStart + (TraceDirection * TraceSettings.ReachDistance);
	float CapsuleRadius = TraceSettings.ForwardTraceRadius;
	float HalfHeight = ((TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2) + 1.0;
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		CapsuleStart,
		CapsuleEnd,
		CapsuleRadius,
		HalfHeight,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		CapsuleActorsToIgnore,
		EDrawDebugTrace::None,
		CapsuleHitResult,
		true
	);
	//判断检测墙体是否为不可行走（墙体角度大于45度为不可行走）或是否击中墙体
	if (!GetCharacterMovement()->IsWalkable(CapsuleHitResult)&&
		CapsuleHitResult.bBlockingHit
		)
	{
		//此处返回值为击中墙体的第一个击中点及其法线
		InitalTrace_ImpactPoint = CapsuleHitResult.ImpactPoint;
		InitalTrace_Normal = CapsuleHitResult.ImpactNormal;
	}
	else
	{
		return false;
	}
	
	//Then1

	/*
	* End为上一步检测击中的第一个点的位置的X,Y值再加上胶囊体基础位置的Z值（0 - 90），然后再加上击中点法线乘以-15的向量（也就是向墙内延伸15cm，防止角色翻墙上去卡在墙边）
	* Start为End再加上（150 + 30 + 1）的Z值高度
	* 所以检测范围为在墙体内部以Start为起点向下延伸181cm以此范围检测墙体是否能够容纳人物在其上运动
	*/

	FHitResult SphereHitResult;
	TArray<AActor*>SphereActorsToIgnore;
	FVector SphereEnd = FVector(InitalTrace_ImpactPoint.X, 
								InitalTrace_ImpactPoint.Y, 
								GetCalpsuleBaseLocation(2.0).Z) + (InitalTrace_Normal * -15.0);
	FVector SphereStart = SphereEnd + FVector(0.0, 0.0, TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius + 1.0);
	float SphereRadius = TraceSettings.DownwardTraceRadius;
	UKismetSystemLibrary::SphereTraceSingle(
		this,
		SphereStart,
		SphereEnd,
		SphereRadius,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		SphereActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		SphereHitResult,
		true
	);
	//判断墙体上方是否可行走，并且判断是否击中墙体上方
	if (GetCharacterMovement()->IsWalkable(SphereHitResult) &&
		SphereHitResult.bBlockingHit
		)
	{
		//此处返回墙体上方的检测球体的垂直Z值及检测击中墙体的点的X,Y值
		DownTraceLocation = FVector(SphereHitResult.Location.X, SphereHitResult.Location.Y, SphereHitResult.ImpactPoint.Z);
		HitComponent = SphereHitResult.GetComponent();
	}
	else
	{
		return false;
	}


	//Then2


	/*
	* 判断墙面上是否能够容纳一整个胶囊体，并返回墙体高度 
	* 此处CapsuleHasRoomCheck检测中，GetCapsuleLocationFormBase返回值为Then2的击中点位置（墙体边缘顶部）再往Z值上加上92的距离
	* 以此为基础位置来进行墙体顶部是否能够容纳一整个胶囊体大小的检测
	* 此处法线的方向原本与人物面向相同，也就是人物攀爬墙体时面向的方向，但是乘以-1则会与人物面向相反
	* 假设攀爬一个墙体，世界坐标为（-1000，2000，0），此时我们原始的法线为（-1，0，0），经过反转则为（1，0，0）
	* 然后再进行从X向量旋转生成FRotator，再和GetCapsuleLocationFormBase返回的Location和Scale（1，1，1）形成一个变换
	* 此时假设我们攀爬的墙体高度为200，也就是DownTraceLocation的Z值高度为200，因为他返回值为顶部位置信息
	* 200 + 92 ,所以GetCapsuleLocationFormBase的Z值返回值为292，所以变换的Location的VectorZ值为292
	* 所以墙体高度（MantleHeight） = 变换的向量值减去Actor世界坐标下的位置然后仅返回Z值（292 - 92）
	* 此时Actor世界坐标位置为刚开始跳跃的第一帧的位置，所以Z值为92（因为计算机计算速度很快）
	*/


	if (CapsuleHasRoomCheck(GetCapsuleComponent(), GetCapsuleLocationFormBase(DownTraceLocation, 2.0), 0.0, 0.0))
	{
		TargetTransform = FTransform(
			(InitalTrace_Normal * FVector(-1.0, -1.0, -1.0)).ToOrientationRotator(),
			GetCapsuleLocationFormBase(DownTraceLocation, 2.0),
			FVector(1.0, 1.0, 1.0)
		);
		MantleHeight = (TargetTransform.GetLocation() - GetActorLocation()).Z;
	}
	else
	{
		return false;
	}

	//Then3

	/*
	* 根据墙高来判断翻越状态
	*/

	//判断角色的运动状态是否不为InAir状态
	if (MovementState == EALSMovementState::EMS_None ||
		MovementState == EALSMovementState::EMS_Grounded ||
		MovementState == EALSMovementState::EMS_Mantling ||
		MovementState == EALSMovementState::EMS_Ragdoll
		)
	{
		//判断墙高是否大于125，高于则返回高翻墙枚举，低于则返回低翻墙枚举
		if (MantleHeight > 125.0)
		{
			MantleType = EALSMantleType::EMT_HighMantle;
		}
		else
		{
			MantleType = EALSMantleType::EMT_LowMantle;
		}

	}
	//若角色当前处于InAir状态，则返回掉落捕获枚举
	else
	{
		MantleType = EALSMantleType::EMT_FallingCatch;
	}

	//Then4

	/*
	* 以上检测均完成后则代表角色当前能够进行攀爬行为
	* 条件：检测到能攀爬的墙体
	* 墙体上方能够供角色行走
	* 墙体上能够容纳一整个胶囊体
	* 枚举有对应的返回值
	*/

	FALSComponentAndTransform LocalComponentAndTransform;
	LocalComponentAndTransform.MantleComponent = HitComponent;
	LocalComponentAndTransform.MantleTransform = TargetTransform;
	MantleStart(LocalComponentAndTransform);
	return true;
}

FVector AALSBaseCharacter::GetCalpsuleBaseLocation(float ZOffset)
{

	/*
	* 将胶囊体锚点位置（根胯骨位置）给他缩减到角色根骨骼位置，未起跳时角色胶囊体的世界坐标Z值为0，起跳为90
	* 为何减去一个ZOffset，是因为一开始设置胶囊体半高为90，默认的半高为88，这边增加的2cm，对应在世界坐标中，胶囊体世界位置的半高也增加了2cm
	* 胶囊体的向上向量始终为单位向量（0.0.1），所以上面的计算结果为（0.0.1),乘以一个90+2等于(0.0.92)
	* 此时我们的胶囊体的世界位置Z值为92，再减去计算后的92,此时角色未起跳时胶囊体的基础位置Z值为0,起跳后Z值为90!!!!!
	* 此处的计算是用于计算攀爬时，角色胶囊体贴合地面的基础位置，而非胶囊体的锚点位置，因为要用来计算地面到墙体最高处的长度
	*/

	return FVector(
		GetCapsuleComponent()->GetComponentLocation() - 
		(GetCapsuleComponent()->GetUpVector() * 
			(GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + ZOffset)
		)
	);

}

FVector AALSBaseCharacter::GetCapsuleLocationFormBase(FVector BaseLocation, float ZOffset)
{
	//返回值为Then2检测的墙体上方的击中点位置（墙体高度）加上（90 + 2）的Z值高度
	return BaseLocation + FVector(0.0, 0.0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + ZOffset);

}

bool AALSBaseCharacter::CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TraceLocation, float HeightOffset, float RadiusOffset)
{

	/*
	* 计算墙体上部是否能够容纳一整个胶囊体
	* 此处获取不带缩放的胶囊体半高（GetScaledCapsuleHalfHeight_WithoutHemisphere）为60
	* 原本整个胶囊体半高为90，去掉头和尾的半圆后就为60
	* RadiusOffset默认值为0，用作不同人物的细节调整，此处为0所以不加入计算
	* HeightOffset默认值为0，也是用作不同人物身高不同的细节调整，不加入计算
	* 所以此处的Start为墙体顶部边缘位置再加上Z值的92距离，然后再加上Z值的60距离，假设是一个200cm高度的墙体，StartZ值大小为（200 + 92 + 60） = 352
	* 所以此处的End为墙体顶部边缘位置再加上Z值的92距离，然后再减去Z值的60距离，假设是一个200cm高度的墙体，StartZ值大小为（200 + 92 - 60） = 232
	* Radius为胶囊体的半径再加上RadiusOffset
	*/
	
	FHitResult HitResult;
	TArray<AActor*>ActorsToIgnore;
	FVector Start = TraceLocation + FVector(
		0.0,
		0.0,
		Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() + (RadiusOffset * -1.0) + HeightOffset
	);
	FVector End = TraceLocation - FVector(
		0.0,
		0.0,
		Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() + (RadiusOffset * -1.0) + HeightOffset
	);
	float Radius = Capsule->GetUnscaledCapsuleRadius() + RadiusOffset;
	UKismetSystemLibrary::SphereTraceSingleByProfile(
		this,
		Start,
		End,
		Radius,
		FName("ALS_Character"),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true
	);
	//判断检测不存在阻挡则返回真
	return !HitResult.bBlockingHit;

}

void AALSBaseCharacter::MantleStart(FALSComponentAndTransform MantleLedgeWS )
{

	//Then0

	/*
	* 将编辑器中重载函数初始化的结构体信息赋值给MantleAsset
	*/

	FMantleAsset MantleAsset = GetMantleAsset(MantleType);

	MantleParams.AnimMontage = MantleAsset.AnimMontage;
	MantleParams.PositionCorrectionCurves = MantleAsset.PositionCorrectionCurves;
	MantleParams.StartingOffset = MantleAsset.StartingOffset;
	//根据墙高的不同，来限制映射高低墙的动画初始位置大小
	MantleParams.StratingPosition = UKismetMathLibrary::MapRangeClamped(
		MantleHeight,
		MantleAsset.LowHeight,
		MantleAsset.HighHeight,
		MantleAsset.LowStartPosition,
		MantleAsset.HighStartPosition
	);
	//根据墙高的不同，来限制映射高低墙的动画播放率大小
	MantleParams.PlayRate = UKismetMathLibrary::MapRangeClamped(
		MantleHeight,
		MantleAsset.LowHeight,
		MantleAsset.HighHeight,
		MantleAsset.LowPlayRate,
		MantleAsset.HighPlayRate
	);

	//Then1

	/*
	* 将角色位于墙体上方的锚点(墙体的X,墙体的Y,墙体高度 + 角色胶囊体半高)数值信息从世界坐标系中转化为局部坐标系
	*/

	MantleLedgeLS.MantleComponent = MantleLedgeWS.MantleComponent;
	MantleLedgeLS.MantleTransform = MantleLedgeWS.MantleTransform *
									MantleLedgeWS.MantleComponent->GetComponentToWorld().Inverse();


	//Then2

	/*
	* 将Actor位于世界坐标上的变换减去角色攀爬时的世界坐标的变换
	* 就获得角色世界坐标系下的实际偏移量
	*/

	MantleTarget = MantleLedgeWS.MantleTransform;

	MantleActualStartOffset = FTransform(
		GetActorTransform().GetRotation() - MantleTarget.GetRotation(),
		GetActorTransform().GetLocation() - MantleTarget.GetLocation(),
		GetActorTransform().GetScale3D() - MantleTarget.GetScale3D()
	);

	//Then3

	/*
	* 此处Params的向量值为（0，65，200）
	* 将角色攀爬时的世界坐标的变换的Location减去(0，65，200)
	* 然后将Location和MantleTarget的rotation组合成变换减去MantleTarget
	*/
	

	MantleAnimatedStartOffset = FTransform(
		MantleTarget.GetRotation() - MantleTarget.GetRotation(),
		(MantleTarget.GetLocation() -
			FVector
			(
				(MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y).X,
				(MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y).Y,
				MantleParams.StartingOffset.Z
			)
		) - MantleTarget.GetLocation(),
		FVector(1.0, 1.0, 1.0) - MantleTarget.GetScale3D()
	);


	//Then4

	/*
	* 修改运动模式判断人物运动，并更改人物状态为正在翻墙
	* 调用接口函数来更新角色的动画蒙太奇枚举值
	*/

	IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	ALSCharacterInterface->SetMovementState(EALSMovementState::EMS_Mantling);


	//Then5

	/*
	* 通过修改时间轴实现翻墙
	*/

	float CurveMinTime;
	float CurveMaxTime;
	MantleParams.PositionCorrectionCurves->GetTimeRange(CurveMinTime, CurveMaxTime);

	//将攀爬曲线的最大值减去开始时间然后将其设置为时间轴长度
	MantleTimeline->SetTimelineLength(
		CurveMaxTime - MantleParams.StratingPosition
	);
	//设置时间轴的播放速率
	MantleTimeline->SetPlayRate(
		MantleParams.PlayRate
	);
	//播放时间轴
	MantleTimeline->PlayFromStart();

	//Then6

	/*
	* 播放动画蒙太奇
	*/

	//判断动画蒙太奇资产是否有效
	if (IsValid(MantleParams.AnimMontage))
	{
		//判断动画实例是否有效
		if (IsValid(MainAnimInstance))
		{
			//播放攀爬动画蒙太奇
			GetMesh()->GetAnimInstance()->Montage_Play(
				MantleParams.AnimMontage,
				MantleParams.PlayRate,
				EMontagePlayReturnType::MontageLength,
				MantleParams.StratingPosition,
				false
			);
		}
	}

}

void AALSBaseCharacter::MantleUpdate(float BlendIn)
{
	
	//Then0

	/*
	* 将角色攀爬锚点参数值从局部坐标系转换为世界坐标系
	*/

	MantleTarget = MantleLedgeLS.MantleTransform * MantleLedgeLS.MantleComponent->GetComponentToWorld();


	//Then1

	/*
	* 利用时间轴当前播放位置加上开始时间作为InTime
	* 获取攀爬曲线的各个轴值的参数值
	* X轴值为角色位置的插值值
	* Y轴为对X,Y校正的插值值
	* Z轴为对Z校正的插值值
	*/
	
	float PositionAlpha = MantleParams.PositionCorrectionCurves->GetVectorValue(
		MantleParams.StratingPosition +
		MantleTimeline->GetPlaybackPosition()
	).X;
	
	float XYCorrectionAlpha = MantleParams.PositionCorrectionCurves->GetVectorValue(
		MantleParams.StratingPosition +
		MantleTimeline->GetPlaybackPosition()
	).Y;

	float ZCorrectionAlpha = MantleParams.PositionCorrectionCurves->GetVectorValue(
		MantleParams.StratingPosition +
		MantleTimeline->GetPlaybackPosition()
	).Z;

	//Then2

	/*
	* 将曲线信息利用Alpha值进行变换过渡
	*/


	const FTransform TLerpActualFormXYAlpha = UKismetMathLibrary::TLerp(
		MantleActualStartOffset,
		FTransform
		(
			MantleAnimatedStartOffset.GetRotation(),
			FVector
			(
				MantleAnimatedStartOffset.GetLocation().X,
				MantleAnimatedStartOffset.GetLocation().Y,
				MantleActualStartOffset.GetLocation().Z
			),
			FVector(1.0, 1.0, 1.0)
		),
		XYCorrectionAlpha
	);



	const FTransform TLerpActualFormZAlpha = UKismetMathLibrary::TLerp(
		MantleActualStartOffset,
		FTransform
		(
			MantleActualStartOffset.GetRotation(),
			FVector
			(
				MantleActualStartOffset.GetLocation().X,
				MantleActualStartOffset.GetLocation().Y,
				MantleAnimatedStartOffset.GetLocation().Z
			),
			FVector(1.0,1.0,1.0)
		),
		ZCorrectionAlpha
	);



	const FTransform LerpMakeTransform = FTransform(
		TLerpActualFormXYAlpha.GetRotation(),
		FVector
		(
			TLerpActualFormXYAlpha.GetLocation().X,
			TLerpActualFormXYAlpha.GetLocation().Y,
			TLerpActualFormZAlpha.GetLocation().Z
		),
		FVector(1.0, 1.0, 1.0)
	);

	const FTransform LerpPosition = UKismetMathLibrary::TLerp
	(
		FTransform
		(
			MantleTarget.GetRotation() + LerpMakeTransform.GetRotation(),
			MantleTarget.GetLocation() + LerpMakeTransform.GetLocation(),
			MantleTarget.GetScale3D() + LerpMakeTransform.GetScale3D()
		),
		MantleTarget,
		PositionAlpha
	);

	FTransform LerpTarget = UKismetMathLibrary::TLerp
	(
		FTransform
		(
			MantleTarget.GetRotation() + MantleActualStartOffset.GetRotation(),
			MantleTarget.GetLocation() + MantleActualStartOffset.GetLocation(),
			MantleTarget.GetScale3D() + MantleActualStartOffset.GetScale3D()
		),
		LerpPosition,
		MantleTimelineCurve->GetFloatValue(MantleTimeline->GetPlaybackPosition())
	);


	//Then3

	/*
	* 将变换过渡后的值赋值给SetActorLocationAndRotation
	* 以此来在时间轴期间内，实时更新角色的位置和旋转值
	*/

	TargetRotation = LerpTarget.GetRotation().Rotator();

	SetActorLocationAndRotation(
		LerpTarget.GetLocation(),
		LerpTarget.GetRotation().Rotator(),
		false,
		false
	);
}

void AALSBaseCharacter::MantleEnd()
{

	/*
	* 时间轴播放完毕后调用此函数
	* 将角色的运动模式设置为行走
	*/

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);


}

void AALSBaseCharacter::BreakfallEvent()
{
	if (IsValid(MainAnimInstance))
	{
		GetMesh()->GetAnimInstance()->Montage_Play(
			GetRollAnimation(),
			1.35
		);
	}

}

void AALSBaseCharacter::RollEvent()
{
	if (IsValid(MainAnimInstance))
	{
		GetMesh()->GetAnimInstance()->Montage_Play(
			GetRollAnimation(),
			1.15
		);
	}
}

UAnimMontage* AALSBaseCharacter::GetRollAnimation()
{
	return nullptr;
}

void AALSBaseCharacter::OnLandFrictionReset()
{
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
}

void AALSBaseCharacter::BreakfallReset()
{
	bBreakfall = false;
}

void AALSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	//事件开始运行时
	OnBeginPlay();

	//若构造函数中创建的时间轴指针为空，则使用此办法创建时间轴
	MantleTimeline = NewObject<UTimelineComponent>(this);
	MantleTimeline->RegisterComponent();

	/*
	* 将相关函数绑定至目标时间轴当中
	* 同时将时间轴曲线之添加至其中
	*/
	
	
	FOnTimelineFloat TimelineUpdated;
	FOnTimelineEvent TimelineFinished;
	TimelineUpdated.BindUFunction(this, FName("MantleUpdate"));
	TimelineFinished.BindUFunction(this, FName("MantleEnd"));
	MantleTimeline->SetTimelineFinishedFunc(TimelineFinished);
	MantleTimeline->SetLooping(false);
	MantleTimeline->SetTimelineLengthMode(TL_TimelineLength);
	MantleTimeline->AddInterpFloat(MantleTimelineCurve, TimelineUpdated);


	//设置玩家控制器
	PlayerController = Cast<APlayerController>(GetController());
	//判断玩家控制器是否为空
	if (IsValid(PlayerController))
	{
		//设置增强输入本地玩家子系统
		EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		//判断增强输入本地玩家子系统是否为空
		if (EnhancedInputLocalPlayerSubsystem)
		{
			//每次添加映射先清除所有映射，避免重复添加
			EnhancedInputLocalPlayerSubsystem->ClearAllMappings();
			EnhancedInputLocalPlayerSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AALSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Then0

	SetEssentialValue();

	//判断角色状态是否为地面状态
	if (MovementState == EALSMovementState::EMS_Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation();
	}
	//判断角色状态是否处于空中
	else if (MovementState == EALSMovementState::EMS_InAir)
	{
		UpdateInAirRotation();
		//在空中有移动输入才能进行攀爬
		if (bHasMovementInput)
		{
			//进行攀爬检测
			MantleCheck(FallingTraceSettings, ETraceTypeQuery::TraceTypeQuery1);
		}
	}

	//Then1

	CacheValues();

	//Then2



	//Then3



}

void AALSBaseCharacter::BlueprintOnJumped()
{
	//当角色跳跃时，调用此事件，更改枚举值/布尔值，并用于状态机中，从而实现跳跃动画

	InAirRotation = UKismetMathLibrary::SelectRotator(
		LastVelocityRotation,
		GetActorRotation(),
		Speed > 100.0
	);
	//判断动画实例是否有效
	if (IsValid(MainAnimInstance))
	{
		//利用接口函数来访问动画实例中的Jumed函数
		IALSAnimationInterface* ALSAnimationInterface = Cast<IALSAnimationInterface>(MainAnimInstance);
		ALSAnimationInterface->Jumped();
	}

}

void AALSBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{

	/*
	* 当运动模式更改时，通过接口的方式，修改人物的状态信息
	*/

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
	//判断当前角色运动状态是否符合要求
	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking ||
		GetCharacterMovement()->MovementMode == EMovementMode::MOVE_NavWalking
		)
	{
		ALSCharacterInterface->SetMovementState(EALSMovementState::EMS_Grounded);
	}
	else if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling)
	{
		ALSCharacterInterface->SetMovementState(EALSMovementState::EMS_InAir);
	}
}

void AALSBaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	/*
	* 通过接口函数来更改角色的步态枚举值
	* 将其设置为蹲伏状态
	*/
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
	ALSCharacterInterface->SetStance(EALSStance::ES_Crouching);
}

void AALSBaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	/*
	* 通过接口函数来更改角色的步态枚举值
	* 将其设置为站立状态
	*/
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	IALSCharacterInterface* ALSCharacterInterface = Cast<IALSCharacterInterface>(this);
	ALSCharacterInterface->SetStance(EALSStance::ES_Standing);

}

void AALSBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (bBreakfall)
	{
		BreakfallEvent();
	}
	else
	{
		if (bHasMovementInput)
		{
			GetCharacterMovement()->BrakingFrictionFactor = 0.5;
			GetWorldTimerManager().SetTimer(OnLandedTimer, this,
				&AALSBaseCharacter::OnLandFrictionReset, 0.5f, false);
		}
		else
		{
			GetCharacterMovement()->BrakingFrictionFactor = 3.0;
			GetWorldTimerManager().SetTimer(OnLandedTimer, this,
				&AALSBaseCharacter::OnLandFrictionReset, 0.5f, false);
		}
	}
	
}

void AALSBaseCharacter::SetEssentialValue()
{

	//Then0
	
	/*
	* 计算每一帧加速度变化量大小
	*/

	Acceleration = CalculateAcceleration();

	//Then1

	/*
	* 将XOY面上的速度储存起来，然后储存速度方向的旋转值
	*/

	//仅获取角色在X,Y面上的速度大小
	Speed = UKismetMathLibrary::Vector4_Size(FVector(
		GetVelocity().X,
		GetVelocity().Y,
		0.0F
	));
	bIsMoving = Speed > 1.0f;
	//该布尔值为真则角色当前是有速度大小
	if (bIsMoving)
	{
		/*
		* 从X向量旋转释义
		* 若有速度大小，则该旋转体记录的是最后一处速度大小的旋转值大小
		* 也就是世界坐标下速度的朝向，与按下的轴值有关，按下wd则yaw为45，按下d为90，按下w为0
		* 计算世界坐标中x轴方向旋转至速度方向后与原x轴方向之间的旋转体大小
		*/
		LastVelocityRotation = GetVelocity().ToOrientationRotator();
	}

	//Then2

	/*
	* 判断有无运动输入，然后储存最后运动输入的方向的对应旋转值
	*/

	MovementInputAmount = 
		UKismetMathLibrary::Vector4_Size(GetCharacterMovement()->GetCurrentAcceleration()) / 
		GetCharacterMovement()->GetMaxAcceleration();
	bHasMovementInput = MovementInputAmount > 0;
	//判断是否有运动输入
	if(bHasMovementInput)
	{
		/*
		* 从X向量旋转释义
		* 若有速度大小，则该旋转体记录的是最后一处速度大小的旋转值大小
		* 也就是世界坐标下速度的朝向，与按下的轴值有关，按下wd则yaw为45，按下d为90，按下w为0
		* 计算世界坐标中x轴方向旋转至速度方向后与原x轴方向之间的旋转体大小
		*/
		LastAccelerationRotation = GetCharacterMovement()->GetCurrentAcceleration().ToOrientationRotator();
	}

	//Then3

	/*
	* 计算每一帧控制器旋转的速度大小
	* 也就是人物摄像机在场景中移动时的速度大小
	*/

	AimYawRate =
		(GetControlRotation().Yaw - PreviousAimYaw) /
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
}

FVector AALSBaseCharacter::CalculateAcceleration()
{
	//计算角色每一帧的加速度大小
	return (GetVelocity() - PreviousVelocity) / UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

}

void AALSBaseCharacter::CacheValues()
{

	/*
	* 计算角色上一帧的数值
	*/

	//Then0
	//获取角色上一帧的速度
	PreviousVelocity = GetVelocity();

	//Then1
	//获取角色上一帧的控制器旋转Yaw值
	PreviousAimYaw = GetControlRotation().Yaw;
}

/*
* PlayerInputGraph
*/

void AALSBaseCharacter::RagdollStart()
{
}

void AALSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//设置增强输入组件
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	//判断增强输入组件是否为空
	if (IsValid(EnhancedInputComponent))
	{
		//前向/后向移动操作
		EnhancedInputComponent->BindAction(IA_ForwardMovementAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::ForwardMovementAction);
		//左向/右向移动操作
		EnhancedInputComponent->BindAction(IA_RightMovementAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::RightMovementAction);
		//向上/向下视口移动操作
		EnhancedInputComponent->BindAction(IA_LookUpAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::LookUpAction);
		//向右/向左视口移动操作
		EnhancedInputComponent->BindAction(IA_LookRightAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::LookRightAction);
		//跳跃操作
		EnhancedInputComponent->BindAction(IA_JumpAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::JumpAction);
		//行走操作
		EnhancedInputComponent->BindAction(IA_WalkAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::WalkAction);
		//冲刺操作
		EnhancedInputComponent->BindAction(IA_SprintAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::SprintAction);
		//蹲伏操作
		EnhancedInputComponent->BindAction(IA_CrouchAction, ETriggerEvent::Triggered, this, &AALSBaseCharacter::CrouchAction);

	}
}

void AALSBaseCharacter::PlayerMovementInput(bool bIsForwardAxis , float Value)
{
	//判断角色状态枚举是否处于地面或者控制，不处于则不能有运动输入
	if (MovementState == EALSMovementState::EMS_Grounded ||
		MovementState == EALSMovementState::EMS_InAir)
	{
		//判断是否是前向移动
		if (bIsForwardAxis)
		{
			//添加前/后向移动输入
			AddMovementInput(
				UKismetMathLibrary::GetForwardVector(
					FRotator(
						0.0f,
						GetControlRotation().Yaw,
						0.0f
					)),
				Value
			);
		}
		else
		{
			//添加右/左向移动输入
			AddMovementInput(
				UKismetMathLibrary::GetRightVector(
					FRotator(
						0.0f,
						GetControlRotation().Yaw,
						0.0f
					)),
				Value
			);
		}
	}

}

void AALSBaseCharacter::ForwardMovementAction(const FInputActionValue& Value)
{
	PlayerMovementInput(true, Value.GetMagnitude());
}

void AALSBaseCharacter::RightMovementAction(const FInputActionValue& Value)
{
	PlayerMovementInput(false, Value.GetMagnitude());
}

void AALSBaseCharacter::LookUpAction(const FInputActionValue& Value)
{
	//添加控制器Pitch值输入值
	AddControllerPitchInput(
		Value.GetMagnitude() * LookUpDownRate
	);
}

void AALSBaseCharacter::LookRightAction(const FInputActionValue& Value)
{
	//添加控制器Yaw值输入值
	AddControllerYawInput(
		Value.GetMagnitude() * LookRightLeftRate
	);
}

void AALSBaseCharacter::JumpAction(const FInputActionValue& Value)
{
	//判断是否持续按着跳跃键
	if (Value.Get<bool>())
	{
		//判断角色当前是否有实现蒙太奇动画
		if (MovementAction == EALSMovementAction::EMA_None)
		{
			//粗判断
			if (MovementState == EALSMovementState::EMS_None ||
				MovementState == EALSMovementState::EMS_Grounded ||
				MovementState == EALSMovementState::EMS_InAir)
			{
				//判断角色是否在地面
				if (MovementState == EALSMovementState::EMS_Grounded)
				{
					if (bHasMovementInput)
					{
						//判断是否不能进行翻墙
						if (!MantleCheck(GroundedTraceSettings, ETraceTypeQuery::TraceTypeQuery1))
						{
							//判断角色当前步态是否为站立状态
							if (Stance == EALSStance::ES_Standing)
							{
								Jump();
							}
							else
							{
								UnCrouch();
							}
						}


					}
					else
					{
						//判断角色当前步态是否为站立状态
						if (Stance == EALSStance::ES_Standing)
						{
							Jump();
						}
						else
						{
							StopJumping();
						}
					}
				}
				//若角色当前跳跃后处于空中状态，则进行翻墙检测
				else if (MovementState == EALSMovementState::EMS_InAir)
				{
					MantleCheck(FallingTraceSettings, ETraceTypeQuery::TraceTypeQuery1);
				}
			}
		}
	}
	//松开跳跃键
	else
	{
		//无输入布尔值则停止跳跃
		StopJumping();
	}
}

void AALSBaseCharacter::WalkAction(const FInputActionValue& Value)
{
	//判断角色期望步态是否处于行走
	if (DesiredGait == EALSGait::EG_Walking)
	{
		DesiredGait = EALSGait::EG_Running;
	}
	//判断角色期望步态是否处于冲刺
	else if (DesiredGait == EALSGait::EG_Running)
	{
		DesiredGait = EALSGait::EG_Walking;
	}
}

void AALSBaseCharacter::SprintAction(const FInputActionValue& Value)
{
	//判断是否持续按着冲刺键
	if (Value.Get<bool>())
	{
		DesiredGait = EALSGait::EG_Sprinting;
	}
	//松开冲刺键
	else
	{
		DesiredGait = EALSGait::EG_Running;
	}
}

void AALSBaseCharacter::CrouchAction(const FInputActionValue& Value)
{

	//判断是否持续按着蹲伏键
	if (Value.Get<bool>())
	{
		//判断角色当前有无进行蒙太奇动画播放
		if (MovementAction == EALSMovementAction::EMA_None)
		{
			UWorld* World = GetWorld();
			check(World);

			const float PrevStanceInputTime = LastStanceInputTime;
			LastStanceInputTime = World->GetTimeSeconds();

			//Roll
			if (LastStanceInputTime - PrevStanceInputTime <= RollDoubleTapTimeout)
			{
				RollEvent();
				//判断角色步态
				if (Stance == EALSStance::ES_Standing)
				{
					DesiredStance = EALSStance::ES_Crouching;
				}
				else
				{
					DesiredStance = EALSStance::ES_Standing;
				}
			}

			//判断角色当前是否处于地面状态
			if (MovementState == EALSMovementState::EMS_Grounded)
			{
				//判断角色步态
				if (Stance == EALSStance::ES_Standing)
				{
					DesiredStance = EALSStance::ES_Crouching;
					Crouch();
				}
				else
				{
					DesiredStance = EALSStance::ES_Standing;
					UnCrouch();
				}
			}
			else if (MovementState == EALSMovementState::EMS_InAir)
			{
				bBreakfall = true;
				GetWorldTimerManager().SetTimer(BreakfallTimer, this,
					&AALSBaseCharacter::BreakfallReset, 0.4f, false);
			}
		}
	}

}

float AALSBaseCharacter::GetAnimCurvesValue(FName CurvesName)
{
	//判断动画实例是否有效，有效则获取动画实例中的曲线向量值
	if (IsValid(MainAnimInstance))
	{
		return MainAnimInstance->GetCurveValue(CurvesName);
	}
	else
	{
		return 0;
	}
		
}
