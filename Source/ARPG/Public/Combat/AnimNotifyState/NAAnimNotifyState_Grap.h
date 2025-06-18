// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NAAnimNotifyState_Grap.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotifyState_Grap : public UAnimNotifyState
{
	GENERATED_BODY()
	
	// 시작
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	// 끝
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// 확인
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	bool SuccessGrab = false;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float OverlapInterval = 0.01f;
	float OverlapElapsed;

	// 잡기 성공 여부로 넣을 collider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName SocketName;
	//당할때 Player가 있어야 하는 Socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName OffsetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float SphereRadius = 20.f;

	//Grap 하는 montage 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* Grabing;

	//Grap 당할때 montage 변경이 필요한경우
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrabedMontage;

	UPROPERTY()
	TSet<AActor*> AppliedActors;

	FGameplayEffectContextHandle ContextHandle;

	FGameplayEffectSpecHandle SpecHandle;
};
