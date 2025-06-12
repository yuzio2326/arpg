// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/Actor/NAProjectileActor.h"
#include "Skill/DataTable/SkillTableRow.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Monster/Pawn/MonsterBase.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ANAProjectileActor::ANAProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    RootComponent = StaticMeshComponent;
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

    //default speed setting
    ProjectileMovementComponent->InitialSpeed = 100.0;
    ProjectileMovementComponent->MaxSpeed = 10000.0;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0;
    InitialLifeSpan = 5.f;


    ProjectileMovementComponent->UpdatedComponent = StaticMeshComponent;
    StaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
}

// Called when the game starts or when spawned
void ANAProjectileActor::BeginPlay()
{
	Super::BeginPlay();



	
}

void ANAProjectileActor::SetProjectileData(const FDataTableRowHandle& InDataTableRowHandle)
{
    DataTableRowHandle = InDataTableRowHandle;
    if (DataTableRowHandle.IsNull()) { return; }
    FProjectileTable* Data = DataTableRowHandle.GetRow<FProjectileTable>(TEXT("Projectile"));
    if (!Data) { ensure(false); return; }

    ProjectileData = Data;


    //Change Default data
    ProjectileMovementComponent->InitialSpeed = ProjectileData->Speed;
    ProjectileMovementComponent->MaxSpeed = ProjectileData->Speed;
    ProjectileMovementComponent->ProjectileGravityScale = ProjectileData->ProjectileGravityScale;
    InitialLifeSpan = ProjectileData->InitialLifeSpan;


    //default Damage 입니다
    GetInstigator()->GetController();


    StaticMeshComponent->MoveIgnoreActors.Empty();
    StaticMeshComponent->MoveIgnoreActors.Add(GetOwner());

    StaticMeshComponent->SetStaticMesh(Data->StaticMesh);
    StaticMeshComponent->SetRelativeTransform(Data->Transform);
}

void ANAProjectileActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (HasAuthority())
    {
        FVector Location = GetOwner()->GetActorLocation();
        if (!IsValid(this)) { return; }

        // BeginPlay 시점에 Overlapped 되면 들어 옴
        if (!bFromSweep)
        {
            Destroy();
            return;
        }
        FTransform NewTransform;
        NewTransform.SetLocation(SweepResult.ImpactPoint);
        FRotator Rotation = UKismetMathLibrary::Conv_VectorToRotator(SweepResult.ImpactNormal);
        NewTransform.SetRotation(Rotation.Quaternion());

        ANACharacter* OwnerPlayer = Cast<ANACharacter>(GetInstigator());
        if (OwnerPlayer)
        {

        }

    }


}

// Called every frame
void ANAProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

