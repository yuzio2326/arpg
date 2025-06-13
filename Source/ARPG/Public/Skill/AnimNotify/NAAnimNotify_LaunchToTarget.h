// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NAAnimNotify_LaunchToTarget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotify_LaunchToTarget : public UAnimNotify
{
	GENERATED_BODY()
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	void LaunchForward();
	void LaunchTarget(USkeletalMeshComponent* MeshComp, FVector LaunchVelocity, bool bXYOverride, bool bZOverride);

	FRotator GetPlayerProjectileRotation(const FVector& InProjectSpawnLocation, class UCameraComponent* InCameraComponent);
	FRotator GetPawnProjectileRotation(const FVector& InProjectSpawnLocation, class USkeletalMeshComponent* InSkeletalMeshComponent);

	
	
};
