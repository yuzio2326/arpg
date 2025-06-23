// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NAAnimNotifyState_QuickDash.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotifyState_QuickDash : public UAnimNotifyState
{
	GENERATED_BODY()
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;


	void LaunchTarget(USkeletalMeshComponent* MeshComp, FVector LaunchVelocity, bool bXYOverride, bool bZOverride);
	bool CheckFalling();
	
	// 데미지 주려고 할때
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	bool bIsDamageSkill;
	// 데미지 주려고 할때 박아놓을 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName SocketName;
	// 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float SphereRadius = 20.f;
	// damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float Damage = 10;
	// 날아가는 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float LaunchForce = 1000.0f;
	// 맞은 대상 위로 날리는 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float TargetLaunchForceZ = 300.0f;
	// 추적기능 (닿으면 꺼짐)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	bool Tracing = false;

	float OverlapInterval = 0.03f;
	float OverlapElapsed;

	UPROPERTY()
	TSet<AActor*> AppliedActors;

	FGameplayEffectContextHandle ContextHandle;

	FGameplayEffectSpecHandle SpecHandle;
	
};
