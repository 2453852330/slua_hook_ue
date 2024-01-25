// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();


	// 此时,才能找到所有的 UFUNCTION
	// TArray<FName> HookFuncList = {
	// 	TEXT("ReceiveBeginPlay") , TEXT("CF_01") , TEXT("CF_02") , TEXT("BP_01") , TEXT("BP_02") ,  TEXT("BP_03")
	// };
	//
	// for (FName & it : HookFuncList)
	// {
	// 	UFunction* func = FindFunction(it);
	// 	UE_LOG(LogTemp, Warning, TEXT("[MyActor->BeginPlay] find func [%s] result [%s]"),*it.ToString(),func?TEXT("TRUE"):TEXT("FALSE"));
	// }
	
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString AMyActor::BPI_GetFilePath_Implementation()
{
	return TEXT("hello world");
}

void AMyActor::CF_01()
{
	UE_LOG(LogTemp, Warning, TEXT("[MyActor->CF_01] this is Empty UFUNCTION func"));
}

void AMyActor::CF_02()
{
	UE_LOG(LogTemp, Warning, TEXT("[MyActor->CF_02] this is BlueprintCallable func"));
}

void AMyActor::BP_03_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[MyActor->BP_03] this is BlueprintNativeEvent func"));
}

