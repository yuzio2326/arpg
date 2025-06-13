// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/AnimNotify/NAAnimNotify_LaunchToTarget.h"
#include "Monster/AI/MonsterAIController.h"
#include "GameFramework/FloatingPawnMovement.h"

void UNAAnimNotify_LaunchToTarget::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// 사용한게 몬스터면 
	if (AMonsterAIController* OwnerMonsterController = Cast<AMonsterAIController>(MeshComp->GetOwner()->GetInstigatorController()))
	{
		if (UBlackboardComponent* MonsterAIBB = OwnerMonsterController->GetBlackboardComponent())
		{
			if (UObject* DetectedPlayer = MonsterAIBB->GetValueAsObject(TEXT("DetectPlayer")))
			{
				if (AActor* DetectedPlayerActor = Cast<AActor>(DetectedPlayer))
				{
					if (APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner()))
					{
						FVector DetectedPlayerLocation = DetectedPlayerActor->GetActorLocation();
						FVector OwnerLocation = MeshComp->GetOwner()->GetActorLocation();
						FVector LaunchDirection = (DetectedPlayerLocation - OwnerLocation).GetSafeNormal(); // 목표 방향 벡터
						float LaunchPower = 1000.0f; // 힘의 크기
						FVector LaunchVelocity = LaunchDirection * LaunchPower;
						LaunchVelocity.Z = 1000.f;
						OwnerPawn->GetRootComponent();

						LaunchTarget(MeshComp, LaunchVelocity, true, true);
					}

				}
			}
		}
	}
	else
	{
		LaunchForward();
	}



}

void UNAAnimNotify_LaunchToTarget::LaunchForward()
{
}

void UNAAnimNotify_LaunchToTarget::LaunchTarget(USkeletalMeshComponent* MeshComp, FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	if (APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (UFloatingPawnMovement* Movement = OwnerPawn->FindComponentByClass<UFloatingPawnMovement>())
		{
			FVector FinalVel = LaunchVelocity;
			const FVector Velocity = Movement->Velocity;

			if (!bXYOverride)
			{
				FinalVel.X += Velocity.X;
				FinalVel.Y += Velocity.Y;
			}
			if (!bZOverride)
			{
				FinalVel.Z += Velocity.Z;
			}
			Movement->Velocity = FinalVel;
		}
	}	
}
