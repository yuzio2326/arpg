// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NAAnimNotify_Projectile.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotify_Projectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	UNAAnimNotify_Projectile();

	FRotator GetPlayerProjectileRotation(const FVector& InProjectSpawnLocation, class UCameraComponent* InCameraComponent);
	FRotator GetPawnProjectileRotation(const FVector& InProjectSpawnLocation, class USkeletalMeshComponent* InSkeletalMeshComponent);

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	// Skilldata 참조해서 만들려고 했던거 시간 적어서 주석처리 -> 나중에 주석 풀고 추가 작업 해야 할거 같음
	//float OwnerDamage;
	
	
	
};
