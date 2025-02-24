// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit_BPFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

bool UUnit_BPFunctionLibrary::MoveUnitWithSteering(AActor* UnitActor, FVector TargetLocation, float DeltaTime, float MoveSpeed, float RotationSpeed, float AvoidanceStrength, float TraceDistance)
{
    if (!UnitActor) return false;

    FVector CurrentLocation = UnitActor->GetActorLocation();
    FVector ForwardVector = UnitActor->GetActorForwardVector();
    FVector MoveDirection = (TargetLocation - CurrentLocation).GetSafeNormal();
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    // If the unit is close enough to the target, stop moving and return true
    if (DistanceToTarget < 100.0f) // Threshold to determine arrival
    {
        return true;
    }

    FHitResult ForwardHit, LeftHit, RightHit;
    FVector TraceStart = CurrentLocation + FVector(0, 0, 50);
    FVector ForwardTraceEnd = TraceStart + (ForwardVector * TraceDistance);
    FVector LeftTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector) * TraceDistance);
    FVector RightTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector) * TraceDistance);

    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(UnitActor);
    ECollisionChannel CustomTraceChannel = ECC_GameTraceChannel1;

    bool bHitForward = UnitActor->GetWorld()->LineTraceSingleByChannel(ForwardHit, TraceStart, ForwardTraceEnd, CustomTraceChannel, TraceParams);
    DrawDebugLine(UnitActor->GetWorld(), TraceStart, ForwardTraceEnd, FColor::Red, false, 0.1f);

    bool bHitLeft = UnitActor->GetWorld()->LineTraceSingleByChannel(LeftHit, TraceStart, LeftTraceEnd, CustomTraceChannel, TraceParams);
    DrawDebugLine(UnitActor->GetWorld(), TraceStart, LeftTraceEnd, FColor::Green, false, 0.1f);

    bool bHitRight = UnitActor->GetWorld()->LineTraceSingleByChannel(RightHit, TraceStart, RightTraceEnd, CustomTraceChannel, TraceParams);
    DrawDebugLine(UnitActor->GetWorld(), TraceStart, RightTraceEnd, FColor::Blue, false, 0.1f);

    FRotator DesiredRotation = MoveDirection.Rotation();
    if (bHitForward)
    {
        if (!bHitRight)
        {
            DesiredRotation = (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector)).Rotation();
        }
        else if (!bHitLeft)
        {
            DesiredRotation = (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector)).Rotation();
        }
    }

    FRotator NewRotation = FMath::RInterpTo(UnitActor->GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed);
    UnitActor->SetActorRotation(NewRotation);

    FVector NewLocation = CurrentLocation + (UnitActor->GetActorForwardVector() * MoveSpeed * DeltaTime);
    UnitActor->SetActorLocation(NewLocation);

    return false; // Still moving towards target
}


TArray<bool> UUnit_BPFunctionLibrary::MoveUnitsInFormation(const FVector& TargetLocation, const TArray<AActor*>& Units, float Spacing, float DeltaTime, float MoveSpeed, float RotationSpeed, float AvoidanceStrength, float TraceDistance)
{
    TArray<bool> UnitArrivals; // To store the arrival status of each unit

    if (Units.Num() == 0) return UnitArrivals;

    int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Units.Num())));

    // Calculate the offset required to center the grid around the target location
    FVector GridOffset = FVector((GridSize - 1) * Spacing * 0.5f, (GridSize - 1) * Spacing * 0.5f, 0);

    int32 Index = 0;

    for (int32 Row = 0; Row < GridSize; ++Row)
    {
        for (int32 Col = 0; Col < GridSize; ++Col)
        {
            if (Index >= Units.Num()) return UnitArrivals;

            // Calculate the target location for the current unit, offset to center the grid
            FVector UnitTargetLocation = TargetLocation + FVector(Row * Spacing, Col * Spacing, 0) - GridOffset;

            // Call MoveUnitWithSteering and store the result in the UnitArrivals array
            bool bArrived = MoveUnitWithSteering(Units[Index], UnitTargetLocation, DeltaTime, MoveSpeed, RotationSpeed, AvoidanceStrength, TraceDistance);

            UnitArrivals.Add(bArrived);

            ++Index;
        }
    }

    return UnitArrivals; // Return the array of arrival statuses
}

