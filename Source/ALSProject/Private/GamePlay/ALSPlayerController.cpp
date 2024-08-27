#include "GamePlay/ALSPlayerController.h"
#include "GamePlay/ALSPlayerCameraManager.h"

void AALSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	/*
	* 将玩家控制器与玩家摄像机管理器进行通讯
	* 然后将控制器中继承OnPossess的Pawn传给玩家摄像机管理器
	*/
	AALSPlayerCameraManager* CameraManager = Cast<AALSPlayerCameraManager>(PlayerCameraManager);
	if (IsValid(CameraManager))
	{
		CameraManager->OnPossess(InPawn);
	}
}
