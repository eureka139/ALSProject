#pragma once

#include "ALSEnum.generated.h"

/*
* 角色步态
*/
UENUM(BlueprintType)
enum class EALSGait : uint8
{
	EG_Walking UMETA(DisplayName = "Walking"),		//走路状态
	EG_Running UMETA(DisplayName = "Running"),		//奔跑状态
	EG_Sprinting UMETA(DisplayName = "Sprinting")	//冲刺状态
};

/*
* 角色蒙太奇
*/
UENUM(BlueprintType)
enum class EALSMovementAction : uint8
{
	EMA_None UMETA(DisplayName = "None"),				//无任何蒙太奇
	EMA_LowMantle UMETA(DisplayName = "LowMantle"),		//低翻墙蒙太奇
	EMA_HighMantle UMETA(DisplayName = "HighMantle"),	//高翻墙蒙太奇
	EMA_Rolling UMETA(DisplayName = "Rolling"),			//翻滚蒙太奇
	EMA_GettingUp UMETA(DisplayName = "GettingUp")		//起身蒙太奇
};

/*
* 角色运动状态
*/
UENUM(BlueprintType)
enum class EALSMovementState : uint8
{
	EMS_None UMETA(DisplayName = "None"),			//无任何运动状态
	EMS_Grounded UMETA(DisplayName = "Grounded"),	//地面状态
	EMS_InAir UMETA(DisplayName = "InAir"),			//滞空状态
	EMS_Mantling UMETA(DisplayName = "Mantling"),	//翻墙状态
	EMS_Ragdoll UMETA(DisplayName = "Ragdoll")		//布娃娃状态
};

/*
* 角色叠加状态
*/
UENUM(BlueprintType)
enum class EALSOverlayState : uint8
{
	EOS_Default UMETA(DisplayName = "Default"), //
	EOS_Masculine UMETA(DisplayName = "Masculine"), //
	EOS_Feminine UMETA(DisplayName = "Feminine"), //
	EOS_Injured UMETA(DisplayName = "Injured"), //
	EOS_HandTied UMETA(DisplayName = "HandTied"), //
	EOS_Rifle UMETA(DisplayName = "Rifle"), //
	EOS_Pistol1H UMETA(DisplayName = "Pistol1H"), //
	EOS_Pistol2H UMETA(DisplayName = "Pistol2H"), //
	EOS_Bow UMETA(DisplayName = "Bow"), //
	EOS_Torch UMETA(DisplayName = "Torch"), //
	EOS_Binoculars UMETA(DisplayName = "Binoculars"), //
	EOS_Box UMETA(DisplayName = "Box"), //
	EOS_Barrel UMETA(DisplayName = "Barrel")

};

/*
* 角色旋转模式
*/
UENUM(BlueprintType)
enum class EALSRotationMode : uint8
{
	ERM_VelocityDirection UMETA(DisplayName = "VelocityDirection"), //速度方向
	ERM_LookingDirection UMETA(DisplayName = "LookingDirection"),	//控制器方向
	ERM_Aiming UMETA(DisplayName = "Aiming")						//瞄准方向
};

/*
* 角色姿态
*/
UENUM(BlueprintType)
enum class EALSStance : uint8
{
	ES_Standing UMETA(DisplayName = "Standing"),	//站立状态
	ES_Crouching UMETA(DisplayName = "Crouching")	//蹲伏状态
};

/*
* 角色视角模式
*/
UENUM(BlueprintType)
enum class EALSViewMode : uint8
{
	EVM_ThirdPerson UMETA(DisplayName = "ThirdPerson"), //第三人称视角模式
	EVM_FirstPerson UMETA(DisplayName = "FirstPerson")  //第一人称视角模式
};

/*
* 
*/
UENUM(BlueprintType)
enum class EALSAnimFeatureExample : uint8
{
	EAE_StrideBlending UMETA(DisplayName = "StrideBlending"),		//步距的混合
	EAE_AdditiveBlending UMETA(DisplayName = "AdditiveBlending"),	//叠加态的混合
	EAE_SprintImpulse UMETA(DisplayName = "SprintImpulse")
};

/*
* 
*/
// UENUM(BlueprintType)
// enum class EALSFootstepType : uint8
// {
//
// };

/*
* 地面进入状态枚举
*/
UENUM(BlueprintType)
enum class EALSGroundedEntryState : uint8
{
	EES_None UMETA(DisplayName = "None"), //非翻滚
	EES_Roll UMETA(DisplayName = "Rool")  //翻滚
};

/*
* 轴心骨骼的朝向
*/
UENUM(BlueprintType)
enum class EALSHipsDirection : uint8
{
	EHD_F UMETA(DisplayName = "F"), //前向
	EHD_B UMETA(DisplayName = "B"), //后向
	EHD_RF UMETA(DisplayName = "RF"), //右前
	EHD_RB UMETA(DisplayName = "RB"), //右后
	EHD_LF UMETA(DisplayName = "LF"), //左前
	EHD_LB UMETA(DisplayName = "LB") //左后

};

/*
* 高低墙翻越
*/
UENUM(BlueprintType)
enum class EALSMantleType : uint8
{

	EMT_HighMantle UMETA(DisplayName = "HighMantle"), //
	EMT_LowMantle UMETA(DisplayName = "LowMantle"), //
	EMT_FallingCatch UMETA(DisplayName = "FallingCatch") //

};

/*
* 角色移动方向
*/
UENUM(BlueprintType)
enum class EALSMovementDirection : uint8
{
	EMD_ForWard UMETA(DisplayName = "ForWard"), //前向
	EMD_Right UMETA(DisplayName = "Right"), //右向
	EMD_Left UMETA(DisplayName = "Left"), //左向
	EMD_BackWard UMETA(DisplayName = "BackWard") //后向
};