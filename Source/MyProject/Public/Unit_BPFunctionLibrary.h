// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Unit_BPFunctionLibrary.generated.h"

/**
 * Blueprint function library for unit movement with obstacle avoidance.
 */
UCLASS()
class MYPROJECT_API UUnit_BPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Moves a unit towards a target location while steering to avoid obstacles.
	 * @param UnitActor - The actor to move.
	 * @param TargetLocation - The destination point.
	 * @param DeltaTime - The frame time for smooth movement.
	 * @param MoveSpeed - Movement speed.
	 * @param RotationSpeed - Rotation speed for steering.
	 * @param AvoidanceStrength - Rotation adjustment when an obstacle is detected.
	 * @param TraceDistance - Distance for obstacle detection.
	 */
	UFUNCTION(BlueprintCallable, Category = "RTS Movement")
	static void MoveUnitWithSteering(AActor* UnitActor, FVector TargetLocation, float DeltaTime, float MoveSpeed, float RotationSpeed = 5.0f, float AvoidanceStrength = 45.0f, float TraceDistance = 150.0f);
};
