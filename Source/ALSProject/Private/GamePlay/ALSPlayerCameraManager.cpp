#include "GamePlay/ALSPlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/ALSCameraBehavior.h"
#include "Interface/ALSCameraInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AALSPlayerCameraManager::AALSPlayerCameraManager()
{
	CameraBehavior = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CameraBehavior"));
	CameraBehavior->SetupAttachment(GetRootComponent());

}

void AALSPlayerCameraManager::OnPossess(APawn* NewPawn)
{
	/*
	* 将受控的Pawn和受控玩家控制器信息传给ALSCameraBehavior
	*/
	ControlledPawn = NewPawn;
	UALSCameraBehavior* CameraBegavior = Cast<UALSCameraBehavior>(CameraBehavior->GetAnimInstance());
	if (IsValid(CameraBegavior))
	{
		CameraBegavior->ControlledPawn = ControlledPawn;
		CameraBegavior->PlayerController = GetOwningPlayerController();
	}
}

float AALSPlayerCameraManager::GetCameraBehaviorParam(FName CurvesName)
{
	/*
	* 获取动画实例里修改的曲线信息
	*/
	if (IsValid(CameraBehavior->GetAnimInstance()))
	{
		return CameraBehavior->GetAnimInstance()->GetCurveValue(CurvesName);
	}
	else
	{
		return 0.0f;
	}
}

FVector AALSPlayerCameraManager::CalculateAxisLndependentLag(FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeed)
{
	FRotator CameraRotationYaw = FRotator(0.0f, CameraRotation.Yaw, 0.0f);
	/*
	* 不旋转向量释义(均是根据世界坐标系来衡量)
	* 
	* 根据旋转值来旋转向量，比如向量为（1，0，0），旋转值为（0，90（Yaw），0）
	* 则不旋转向量返回值为（0，-1，0）
	* 旋转向量返回值为（0，1，0）
	* 不旋转向量就是逆时针旋转
	* 旋转向量就是顺时针旋转
	*
	* 此处将摄像机当前位置到目标位置进行数值上的变换，主要体现在将向量信息进行不旋转向量（逆时针方向的旋转），
	* 将其变换至局部坐标系当中，然后再进行一些插值变换之后，再通过旋转向量（顺时针旋转）再将其变回世界坐标系.
	*
	* 在局部坐标系进行数值变化，然后再变回世界坐标系，再将世界坐标系下的返回值传出
	*（局部坐标系下的数值与世界坐标系的数值基本不同)
	*/
	FVector UnRotationCurrentLoc = CameraRotationYaw.UnrotateVector(CurrentLocation);
	FVector UnRotationTargetLoc = CameraRotationYaw.UnrotateVector(TargetLocation);
	FVector InterpVector = UKismetMathLibrary::MakeVector(
		UKismetMathLibrary::FInterpTo(
			UnRotationCurrentLoc.X,
			UnRotationTargetLoc.X,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			LagSpeed.X
		),
		UKismetMathLibrary::FInterpTo(
			UnRotationCurrentLoc.Y,
			UnRotationTargetLoc.Y,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			LagSpeed.Y
		),
		UKismetMathLibrary::FInterpTo(
			UnRotationCurrentLoc.Z,
			UnRotationTargetLoc.Z,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			LagSpeed.Z
		)
	);
	return CameraRotationYaw.RotateVector(InterpVector);
}

void AALSPlayerCameraManager::CustomCameraBehavior(FVector& Location, FRotator& Rotation, float& FOV)
{
	//Then0
	/*
	* 在ALSCharacter中重载接口信息后
	* 将其传递给ALSPlayerCameraManager
	* 锚点信息均取决于动画实例中修改曲线赋值的大小
	*/
	//创建接口函数
	IALSCameraInterface* ALSCameraInterface = Cast<IALSCameraInterface>(ControlledPawn);
	//第三人称摄像机锚点位置(位于根胯骨位置)
	FTransform PivotTarget;
	ALSCameraInterface->GetTPPivotTarget(PivotTarget);
	//第一人才摄像机锚点位置
	FVector FirstPersonTarget;
	ALSCameraInterface->GetFPCameraTarget(FirstPersonTarget);
	//摄像机基础参数值
	float FirstPersonFOV;
	float ThirdPersonFOV;
	bool RightShoulder;
	ALSCameraInterface->GetCameraParameters(ThirdPersonFOV, FirstPersonFOV, RightShoulder);

	//Then1
	/*
	* 让摄像机的旋转值滞后跟随控制器的旋转值
	* 此处摄像机追逐效果需配合时间膨胀使用效果明显(在子弹时间的一些操作效果明显，基于时间膨胀)
	* 锚点信息均取决于动画实例中修改曲线赋值的大小
	*/

	TargetCameraRotation = UKismetMathLibrary::RLerp(
		UKismetMathLibrary::RInterpTo(
			GetCameraRotation(),
			GetOwningPlayerController()->GetControlRotation(),
			UGameplayStatics::GetWorldDeltaSeconds(this),
			GetCameraBehaviorParam(FName("RotationLagSpeed"))
		),
		DebugViewRotation,
		GetCameraBehaviorParam(FName("Override_Debug")),
		true
	);

	//Then2
	/*
	* 从人物蓝图中获取的锚点变换信息通过过渡的方式储存在玩家摄像机管理器，
	* 在锚点的基础上再去进行数值上的变化（主要是Location的变化）
	* 运用曲线值PivotLagSpeed来实现锚点位置的更改
	* 锚点信息均取决于动画实例中修改曲线赋值的大小
	*/

	//从人物骨骼中获取曲线信息
	FVector CurvesVector = UKismetMathLibrary::MakeVector(
		GetCameraBehaviorParam(FName("PivotLagSpeed_X")),
		GetCameraBehaviorParam(FName("PivotLagSpeed_Y")),
		GetCameraBehaviorParam(FName("PivotLagSpeed_Z"))
	);
	//计算各个轴值的滞后速度(主要体现在人物移动时的滞后速度)
	FVector AxisVector = CalculateAxisLndependentLag(
		SmoothTargetPivot.GetLocation(),
		PivotTarget.GetLocation(),
		TargetCameraRotation,
		CurvesVector
	);
	//将计算结果赋值给锚点(此处的锚点信息均是变化的,即按下WASD后可触发，未按下时与PivotTarget的锚点信息相同)
	SmoothTargetPivot.SetRotation(PivotTarget.GetRotation());
	SmoothTargetPivot.SetLocation(AxisVector);
	SmoothTargetPivot.SetScale3D(FVector::OneVector);

	//Then3
	/*
	* 在获取人物锚点位置的基础上，分别在XYZ上进行位置数值增加
	* 运用曲线值PivotOffset来实现锚点位置的更改
	* 以此来进一步控制摄像机
	* 根据角色世界坐标锚点旋转来实时获取角色各个朝向的向量值
	*（该向量值始终基于世界坐标，向量方向不改变）
	* 该向量只有方向没有大小，所以需要乘以一个长度值
	* 锚点信息均取决于动画实例中修改曲线赋值的大小
	*/

	PivotLocation =
		SmoothTargetPivot.GetLocation() +
		UKismetMathLibrary::GetForwardVector(SmoothTargetPivot.Rotator()) * GetCameraBehaviorParam(FName("PivotOffset_X")) +
		UKismetMathLibrary::GetRightVector(SmoothTargetPivot.Rotator()) * GetCameraBehaviorParam(FName("PivotOffset_Y")) +
		UKismetMathLibrary::GetUpVector(SmoothTargetPivot.Rotator()) * GetCameraBehaviorParam(FName("PivotOffset_Z"));

	//Then4
	/*
	* 在人物锚点位置的基础上，再次进行数值上的增加，最终获取了目标摄像机位置
	*/

	FVector CameraLocation =
		PivotLocation +
		UKismetMathLibrary::GetForwardVector(TargetCameraRotation) * GetCameraBehaviorParam(FName("CameraOffset_X")) +
		UKismetMathLibrary::GetRightVector(TargetCameraRotation) * GetCameraBehaviorParam(FName("CameraOffset_Y")) +
		UKismetMathLibrary::GetUpVector(TargetCameraRotation) * GetCameraBehaviorParam(FName("CameraOffset_Z"));
	TargetCameraLocation = UKismetMathLibrary::VLerp(
		CameraLocation,
		PivotTarget.GetLocation() + DebugViewOffset,
		GetCameraBehaviorParam(FName("Override_Debug"))
	);

	//Then5
	/*
	* 使用射线检测来防止摄像机出现穿墙的情况
	*/
	FVector TraceOrigin;
	float TraceRadius;
	ETraceTypeQuery TraceChannel;
	TArray<AActor*>ActorsToIgnore;
	FHitResult HitResult;
	ALSCameraInterface->GetTPTraceParams(TraceOrigin, TraceRadius, TraceChannel);

	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		TraceOrigin,
		TargetCameraLocation,
		TraceRadius,
		TraceChannel,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true
	);

	if (HitResult.IsValidBlockingHit())
	{
		TargetCameraLocation = (HitResult.Location - HitResult.TraceEnd) + TargetCameraLocation;
	}

	//Then6
	/*
	* Debug
	*/

	if (true)
	{
		//绘制根胯骨位置的锚点球体
		UKismetSystemLibrary::DrawDebugSphere(
			GetWorld(),
			PivotTarget.GetLocation(),
			16.0,
			8,
			FColor::Green
		);
		//绘制根胯骨在移动时X轴偏移的锚点球体
		UKismetSystemLibrary::DrawDebugSphere(
			GetWorld(),
			SmoothTargetPivot.GetLocation(),
			16.0,
			8,
			FColor::Orange
		);
		//绘制在根胯骨偏移球体的基础上在进行Z轴偏移的锚点球体
		UKismetSystemLibrary::DrawDebugSphere(
			GetWorld(),
			PivotLocation,
			16.0,
			8,
			FColor::Blue
		);
		//绘制从X轴偏移锚点球体到Z轴偏移锚点球体链接的线条
		UKismetSystemLibrary::DrawDebugLine(
			GetWorld(),
			SmoothTargetPivot.GetLocation(),
			PivotTarget.GetLocation(),
			FColor::Orange
		);
		//绘制从锚点原点球体至X轴偏移锚点球体的链接线条
		UKismetSystemLibrary::DrawDebugLine(
			GetWorld(),
			PivotLocation,
			SmoothTargetPivot.GetLocation(),
			FColor::Blue
		);

	}

	//Then7
	/*
	* 返回摄像机的具体信息并应用于蓝图更新摄像机中
	*/
	FTransform TargetCameraTransform = UKismetMathLibrary::TLerp(
		UKismetMathLibrary::TLerp(
			UKismetMathLibrary::MakeTransform(TargetCameraLocation, TargetCameraRotation),
			UKismetMathLibrary::MakeTransform(FirstPersonTarget, TargetCameraRotation),
			GetCameraBehaviorParam(FName("Weight_FirstPerson"))
		),
		UKismetMathLibrary::MakeTransform(
			TargetCameraLocation,
			DebugViewRotation
		),
		GetCameraBehaviorParam(FName("Override_Debug"))
	);
	Location = TargetCameraTransform.GetLocation();
	Rotation = TargetCameraTransform.Rotator();
	FOV = UKismetMathLibrary::Lerp(
		ThirdPersonFOV,
		FirstPersonFOV,
		GetCameraBehaviorParam(FName("Override_Debug"))
	);
}
