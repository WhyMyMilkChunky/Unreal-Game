// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestActor.generated.h"

UCLASS()
class MYPROJECT_API ATestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BluePrintCallable, Category = "My Functions")
	void TestFunction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
