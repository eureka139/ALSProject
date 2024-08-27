#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALSCameraInterface.generated.h"


UINTERFACE(MinimalAPI)
class UALSCameraInterface : public UInterface
{
	GENERATED_BODY()
};

class ALSPROJECT_API IALSCameraInterface
{
	GENERATED_BODY()


public:
	//获取摄像机的参数值
	virtual void GetCameraParameters(float& TPFOV, float& FPFOV, bool& RightShoulder);
	//获取第一人称摄像机位置
	virtual void GetFPCameraTarget(FVector& ReturnValue);
	//获取第三人称锚点位置
	virtual void GetTPPivotTarget(FTransform& ReturnValue);
	//获取第三人称射线检测参数
	virtual void GetTPTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel);
};
