// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NAAnimNotifyState_Grabbing.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotifyState_Grabbing : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	bool SuccessEscape = false;
	bool EscapeFront = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float OverlapInterval = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float SphereRadius = 20.f;

	//Grab 할때 escape 성공시 montage 변경이 필요한경우
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EscapeMontage;
	//Player가 escape 성공시 사용할 montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PlayerEscapeMontage;
	//Escape 실패시 player에게 damage입힐 montage 변경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EscapeFailedMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PlayerEscapeFailedMontage;

	//Player 붙잡아 놓고 있어야 하는 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName GrabSocketName;
	// 일정 범위 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName MainSocketName;
	//아군 공격으로 탈출가능하게 하기 위해 체크 
	UPROPERTY()
	TSet<AActor*> AppliedActors;

	UPROPERTY()
	TSet<AActor*> GrabbingActors;

	FGameplayEffectContextHandle ContextHandle;

	FGameplayEffectSpecHandle SpecHandle;

};
