// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NAProjectileActor.generated.h"

class UProjectileMovementComponent;
struct FProjectileTable;

UCLASS()
class ARPG_API ANAProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANAProjectileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetProjectileData(const FDataTableRowHandle& InDataTableRowHandle);
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY()
	UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultSceneRoot;
	UPROPERTY(EditAnywhere, meta = (RowType = "/Script/ARPG.ProjectileTable"))
	FDataTableRowHandle DataTableRowHandle;

	FProjectileTable* ProjectileData;
	
};
