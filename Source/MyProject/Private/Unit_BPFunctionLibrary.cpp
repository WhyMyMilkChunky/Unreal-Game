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

    // Early exit if the unit is close enough to the target
    if (DistanceToTarget < 100.0f)
    {
        return true;
    }

    // Setup for trace start
    FVector TraceStart = CurrentLocation + FVector(0, 0, 50);

    // Helper function for performing line traces
    auto PerformLineTrace = [&](FVector Start, FVector End) -> bool
        {
            FHitResult HitResult;
            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(UnitActor);
            ECollisionChannel CustomTraceChannel = ECC_GameTraceChannel1;
            bool bHit = UnitActor->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, CustomTraceChannel, TraceParams);
            return bHit;
        };

    // Perform line traces for forward, left, and right
    FVector ForwardTraceEnd = TraceStart + (ForwardVector * TraceDistance);
    FVector LeftTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector) * TraceDistance);
    FVector RightTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector) * TraceDistance);

    bool bHitForward = PerformLineTrace(TraceStart, ForwardTraceEnd);
    bool bHitLeft = PerformLineTrace(TraceStart, LeftTraceEnd);
    bool bHitRight = PerformLineTrace(TraceStart, RightTraceEnd);

    // Draw debug lines for visualization
    DrawDebugLine(UnitActor->GetWorld(), TraceStart, ForwardTraceEnd, FColor::Red, false, 0.1f);
    DrawDebugLine(UnitActor->GetWorld(), TraceStart, LeftTraceEnd, FColor::Green, false, 0.1f);
    DrawDebugLine(UnitActor->GetWorld(), TraceStart, RightTraceEnd, FColor::Blue, false, 0.1f);

    // Adjust rotation based on avoidance behavior
    FRotator DesiredRotation = MoveDirection.Rotation();
    if (bHitForward)
    {
        // Rotate away from obstacles if forward is blocked
        if (!bHitRight)
        {
            DesiredRotation = (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector)).Rotation();
        }
        else if (!bHitLeft)
        {
            DesiredRotation = (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector)).Rotation();
        }
    }

    // Smooth rotation towards desired direction
    FRotator NewRotation = FMath::RInterpTo(UnitActor->GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed);
    UnitActor->SetActorRotation(NewRotation);

    // Move the actor forward
    FVector NewLocation = CurrentLocation + (ForwardVector * MoveSpeed * DeltaTime);
    UnitActor->SetActorLocation(NewLocation);

    return false; // Still moving towards target
}



TArray<FVector> UUnit_BPFunctionLibrary::GetFormationPositions(const FVector& TargetLocation, const TArray<AActor*>& Units, float Spacing)
{
    TArray<FVector> FormationPositions;
    if (Units.Num() == 0) return FormationPositions;

    int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Units.Num())));

    // Calculate the offset required to center the grid around the target location
    FVector GridOffset = FVector((GridSize - 1) * Spacing * 0.5f, (GridSize - 1) * Spacing * 0.5f, 0);

    int32 Index = 0;

    for (int32 Row = 0; Row < GridSize; ++Row)
    {
        for (int32 Col = 0; Col < GridSize; ++Col)
        {
            if (Index >= Units.Num()) return FormationPositions;

            // Calculate the target location for the current unit, offset to center the grid
            FVector UnitTargetLocation = TargetLocation + FVector(Row * Spacing, Col * Spacing, 0) - GridOffset;
            FormationPositions.Add(UnitTargetLocation);

            ++Index;
        }
    }

    return FormationPositions;
}




