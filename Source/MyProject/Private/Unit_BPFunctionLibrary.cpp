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

    //close enough
    if (DistanceToTarget < 100.0f)
    {
        return true;
    }

    FVector TraceStart = CurrentLocation + FVector(0, 0, 50);

    //check if there’s anything in the way
    auto PerformLineTrace = [&](FVector Start, FVector End) -> bool
        {
            FHitResult HitResult;
            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(UnitActor);
            ECollisionChannel CustomTraceChannel = ECC_GameTraceChannel4;
            bool bHit = UnitActor->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, CustomTraceChannel, TraceParams);
            return bHit;
        };

    //not crashing into walls
    FVector ForwardTraceEnd = TraceStart + (ForwardVector * TraceDistance);
    FVector LeftTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector) * TraceDistance);
    FVector RightTraceEnd = TraceStart + (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector) * TraceDistance);

    bool bHitForward = PerformLineTrace(TraceStart, ForwardTraceEnd);
    bool bHitLeft = PerformLineTrace(TraceStart, LeftTraceEnd);
    bool bHitRight = PerformLineTrace(TraceStart, RightTraceEnd);

    FRotator DesiredRotation = MoveDirection.Rotation();
    if (bHitForward)
    {
        // maybe go left or right
        if (!bHitRight)
        {
            DesiredRotation = (ForwardVector.RotateAngleAxis(AvoidanceStrength, FVector::UpVector)).Rotation();
        }
        else if (!bHitLeft)
        {
            DesiredRotation = (ForwardVector.RotateAngleAxis(-AvoidanceStrength, FVector::UpVector)).Rotation();
        }
    }

	// smooth rotation because abrupt turns are so last century (cmon, it’s 2025 guys get real)
    FRotator NewRotation = FMath::RInterpTo(UnitActor->GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed);
    UnitActor->SetActorRotation(NewRotation);

    //move forward
    FVector NewLocation = CurrentLocation + (ForwardVector * MoveSpeed * DeltaTime);
    UnitActor->SetActorLocation(NewLocation);

    return false;//still getting there
}



TArray<FVector> UUnit_BPFunctionLibrary::GetFormationPositions(const FVector& TargetLocation, const TArray<AActor*>& Units, float Spacing)
{
    TArray<FVector> FormationPositions;
    if (Units.Num() == 0) return FormationPositions;

    int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Units.Num())));

    //offset required to center the grid around the target location
    FVector GridOffset = FVector((GridSize - 1) * Spacing * 0.5f, (GridSize - 1) * Spacing * 0.5f, 0);

    int32 Index = 0;

    for (int32 Row = 0; Row < GridSize; ++Row)
    {
        for (int32 Col = 0; Col < GridSize; ++Col)
        {
            if (Index >= Units.Num()) return FormationPositions;

            //target location for the current unit, offset to center the grid
            FVector UnitTargetLocation = TargetLocation + FVector(Row * Spacing, Col * Spacing, 0) - GridOffset;
            FormationPositions.Add(UnitTargetLocation);

            ++Index;
        }
    }

    return FormationPositions;
}

//attacking targets, because units like to do that
#include "Kismet/GameplayStatics.h" // Required for ApplyDamage

bool UUnit_BPFunctionLibrary::AttackTarget(AActor* Attacker, AActor* Target, float DeltaTime, float AttackRate, float AttackRange,
    FString TargetType, float BaseDamage, float DamageMultiplier, float MoveSpeed, float RotationSpeed, float AvoidanceStrength, float TraceDistance)
{
    if (!Attacker || !Target) return false;

    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    float DistanceToTarget = FVector::Dist(AttackerLocation, TargetLocation);

    // Move within attack range using obstacle avoidance
    if (DistanceToTarget > AttackRange)
    {
        MoveUnitWithSteering(Attacker, TargetLocation, DeltaTime, MoveSpeed, RotationSpeed, AvoidanceStrength, TraceDistance);
        return false; // Still approaching target
    }

    // Ensure attacker faces the target before attacking
    FRotator DesiredRotation = (TargetLocation - AttackerLocation).Rotation();
    Attacker->SetActorRotation(FMath::RInterpTo(Attacker->GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed));

    // Check if the attacker is facing the target before attacking
    FVector ForwardVector = Attacker->GetActorForwardVector();
    FVector DirectionToTarget = (TargetLocation - AttackerLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);

    if (DotProduct < 0.95f) // Not yet facing target, wait before attacking
    {
        return false;
    }

    UWorld* World = Attacker->GetWorld();
    if (!World) return false;

    // Attack cooldown logic
    static TMap<AActor*, float> AttackTimers;
    float& LastAttackTime = AttackTimers.FindOrAdd(Attacker);

    if (World->GetTimeSeconds() - LastAttackTime < AttackRate)
    {
        return false; // Still cooling down
    }

    // Update attack timer
    LastAttackTime = World->GetTimeSeconds();

    // Calculate final damage based on target type
    float FinalDamage = BaseDamage;
    if (TargetType == "Footman")
    {
        FinalDamage *= 1.0f;
    }
    else if (TargetType == "Archer")
    {
        FinalDamage *= 1.2f;
    }
    else if (TargetType == "Cavalry")
    {
        FinalDamage *= 0.8f;
    }

    FinalDamage *= DamageMultiplier;

    // Apply damage using Unreal's built-in system
    UGameplayStatics::ApplyDamage(Target, FinalDamage, Attacker->GetInstigatorController(), Attacker, nullptr);

    return true; // Attack executed
}








