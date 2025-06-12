// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/AnimNotify/NAAnimNotify_Projectile.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NACharacter.h"
#include "Monster/Pawn/MonsterBase.h"
#include "Monster/AI/MonsterAIController.h"
#include "Skill/Actor/NAProjectileActor.h"

UNAAnimNotify_Projectile::UNAAnimNotify_Projectile()
{
}

FRotator UNAAnimNotify_Projectile::GetPlayerProjectileRotation(const FVector& InProjectSpawnLocation, UCameraComponent* InCameraComponent)
{
    const FVector CameraForwardVector = InCameraComponent->GetForwardVector();
    const FVector DestinationLocation = InCameraComponent->GetComponentLocation() + CameraForwardVector * 5000.0;

    TArray<AActor*> IgnoreActors; IgnoreActors.Add(InCameraComponent->GetOwner());
    FHitResult HitResult;

    const ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel4);
    const bool bHit = UKismetSystemLibrary::LineTraceSingle(InCameraComponent->GetWorld(),
        InCameraComponent->GetComponentLocation(), DestinationLocation, TraceTypeQuery,
        false, IgnoreActors, EDrawDebugTrace::None, HitResult, true);

    FRotator Rotator;
    if (bHit)
    {
        Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, HitResult.ImpactPoint);
    }
    else
    {
        Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, DestinationLocation);
    }
    return Rotator;
}

FRotator UNAAnimNotify_Projectile::GetPawnProjectileRotation(const FVector& InProjectSpawnLocation, USkeletalMeshComponent* InSkeletalMeshComponent)
{
    const FVector CameraForwardVector = InSkeletalMeshComponent->GetForwardVector();
    const FVector DestinationLocation = InSkeletalMeshComponent->GetComponentLocation() + CameraForwardVector * 5000.0;

    TArray<AActor*> IgnoreActors; IgnoreActors.Add(InSkeletalMeshComponent->GetOwner());
    FHitResult HitResult;

    const ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel4);
    const bool bHit = UKismetSystemLibrary::LineTraceSingle(InSkeletalMeshComponent->GetWorld(),
        InSkeletalMeshComponent->GetComponentLocation(), DestinationLocation, TraceTypeQuery,
        false, IgnoreActors, EDrawDebugTrace::None, HitResult, true);

    FRotator Rotator;
    if (bHit)
    {
        Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, HitResult.ImpactPoint);
    }
    else
    {
        Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, DestinationLocation);
    }
    return Rotator;
}

void UNAAnimNotify_Projectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);


#if WITH_EDITOR
    if (GIsEditor && MeshComp->GetWorld() != GWorld) { return; } // 에디터 프리뷰
#endif

    AActor* OwningActor = MeshComp->GetOwner();


    // Server 에서 Spawn 할수 있도록
    if (OwningActor->HasAuthority())
    {
        //Socket 기준 location 을 받아서 spawn하도록 하기
        const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
        const FRotator SocketRotator = MeshComp->GetSocketRotation(SocketName);

        FRotator ProjectileRotator = FRotator::ZeroRotator;
        bool bIsPlayer = false;
        //Camera 보유시 Player 아닐시 AI 로 spawn Location Rotation 조절
        if (UCameraComponent* CameraComponent = OwningActor->GetComponentByClass<UCameraComponent>())
        {
            // camera & Socket Location 계산
            ProjectileRotator = GetPlayerProjectileRotation(SocketLocation, CameraComponent);
            //projectile data받아서 사용 해야 하는데... 만들어야 함.. 아니면 기본 데미지로
        }
    }


}

