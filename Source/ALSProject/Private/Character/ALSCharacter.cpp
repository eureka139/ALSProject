#include "Character/ALSCharacter.h"
#include "Kismet/KismetMathLibrary.h"

/*
* 接口函数重载
*/
//获取第三人称左右肩摄像机检测信息
void AALSCharacter::GetTPTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel)
{
	if (bRightShoulder)
	{
		TraceOrigin = GetMesh()->GetSocketLocation(FName("TP_CameraTrace_R"));
		TraceRadius = 10;
		TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
	}
	else
	{
		TraceOrigin = GetMesh()->GetSocketLocation(FName("TP_CameraTrace_L"));
		TraceRadius = 10;
		TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
	}
}
//获取第三人称基础锚点位置
void AALSCharacter::GetTPPivotTarget(FTransform& ReturnValue)
{
	TArray<FVector>SocketLocationArray;
	SocketLocationArray = {
		GetMesh()->GetSocketLocation(FName("head")) ,
		GetMesh()->GetSocketLocation(FName("root"))
	};
	//此处是将head和root的位置加起来除以二,所获取的插槽位置是世界位置
	ReturnValue = UKismetMathLibrary::MakeTransform(UKismetMathLibrary::GetVectorArrayAverage(SocketLocationArray), GetActorRotation());
}
//获取第一人称摄像机位置
void AALSCharacter::GetFPCameraTarget(FVector& ReturnValue)
{
	ReturnValue = GetMesh()->GetSocketLocation(FName("FP_Camera"));
}

void AALSCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AALSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UAnimMontage* AALSCharacter::GetRollAnimation()
{
	
		return LandRollDefault;

	
}
