// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NAAnimNotifyState_ParryAreaTest.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotifyState_ParryAreaTest : public UAnimNotifyState
{
	GENERATED_BODY()
	
	// 시작
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	// 끝
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// 확인
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	bool Check;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float OverlapInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float SphereRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* StunMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* ParrySound = nullptr;

	FGameplayEffectContextHandle ContextHandle;

	FGameplayEffectSpecHandle SpecHandle;

};
