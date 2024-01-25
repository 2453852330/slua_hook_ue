// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "HookSetup.h"



UMyGameInstance::UMyGameInstance()
{
	// 因为编辑器启动时会创建一个 GameInstance 且无法销毁;
	// 防止编辑器启动时自动创建的GameInstance的影响
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		if (hook)
		{
			delete hook;
			hook = nullptr;
		}

		// 创建一个 HOOK 对象
		hook = new HookSetup();
	}
}

void UMyGameInstance::Init()
{
	Super::Init();
}

void UMyGameInstance::Shutdown()
{
	if (hook)
	{
		delete hook;
		hook = nullptr;
	}
	
	Super::Shutdown();
}
