// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyInterface.h"

#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

UCLASS()
class EXAMPLEDEMO426_API AMyActor : public AActor , public IMyInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual FString BPI_GetFilePath_Implementation() override;


	UFUNCTION()
	void CF_01();

	UFUNCTION(BlueprintCallable)
	void CF_02();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_01();

	UFUNCTION(BlueprintNativeEvent)
	void BP_03();
};
