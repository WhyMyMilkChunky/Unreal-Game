// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit_BPFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

void UUnit_BPFunctionLibrary::MoveUnitWithSteering(AActor* UnitActor, FVector TargetLocation, float DeltaTime, float MoveSpeed, float RotationSpeed, float AvoidanceStrength, float TraceDistance)
{
	if (!UnitActor) return;

	// Get current location and direction
	FVector CurrentLocation = UnitActor->GetActorLocation();
	FVector ForwardVector = UnitActor->GetActorForwardVector();
	FVector MoveDirection = (TargetLocation - CurrentLocation).GetSafeNormal();

	// Obstacle Avoidance using Line Traces
	FHitResult ForwardHit, LeftHit, RightHit;
	FVector TraceStart = CurrentLocation + FVector(0, 0, 50); // Slightly above ground to avoid floor hits

	// Trace directions
	FVector ForwardTraceEnd = TraceStart + (ForwardVector * TraceDistance);
	FVector LeftTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector) * TraceDistance);
	FVector RightTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector) * TraceDistance);

	// Collision params (ignores self)
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(UnitActor);

	// **Use ECollisionChannel for custom trace channel**
	ECollisionChannel CustomTraceChannel = ECC_GameTraceChannel1; // "Interactable" should be set as ECC_GameTraceChannel1

	// Perform traces (using "Interactable" custom channel)
	bool bHitForward = UnitActor->GetWorld()->LineTraceSingleByChannel(ForwardHit, TraceStart, ForwardTraceEnd, CustomTraceChannel, TraceParams);
	DrawDebugLine(UnitActor->GetWorld(), TraceStart, ForwardTraceEnd, FColor::Red, false, 0.1f);

	bool bHitLeft = UnitActor->GetWorld()->LineTraceSingleByChannel(LeftHit, TraceStart, LeftTraceEnd, CustomTraceChannel, TraceParams);
	DrawDebugLine(UnitActor->GetWorld(), TraceStart, LeftTraceEnd, FColor::Green, false, 0.1f);

	bool bHitRight = UnitActor->GetWorld()->LineTraceSingleByChannel(RightHit, TraceStart, RightTraceEnd, CustomTraceChannel, TraceParams);
	DrawDebugLine(UnitActor->GetWorld(), TraceStart, RightTraceEnd, FColor::Blue, false, 0.1f);

	// Adjust Rotation if Obstacle is Detected
	FRotator DesiredRotation = MoveDirection.Rotation();
	if (bHitForward)
	{
		// Steer left or right based on which side is clearer
		if (!bHitRight)
		{
			DesiredRotation = (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector)).Rotation();
		}
		else if (!bHitLeft)
		{
			DesiredRotation = (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector)).Rotation();
		}
	}

	// Smooth Rotation
	FRotator NewRotation = FMath::RInterpTo(UnitActor->GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed);
	UnitActor->SetActorRotation(NewRotation);

	// Move Forward
	FVector NewLocation = CurrentLocation + (UnitActor->GetActorForwardVector() * MoveSpeed * DeltaTime);
	UnitActor->SetActorLocation(NewLocation);
}
