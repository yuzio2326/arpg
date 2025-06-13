// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_QuickDash.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Monster/AI/MonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/OverlapResult.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "NACharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UNAAnimNotifyState_QuickDash::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		if (MeshComp->GetOwner()->HasAuthority())
		{
			const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();
			if (!SourceInterface)
			{
				// GAS가 없는 객체로부터 시도됨
				check(false);
				return;
			}
			if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
			{
				const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
				ContextHandle = OwnerASC->MakeEffectContext();
				ContextHandle.AddOrigin(SocketLocation);
				ContextHandle.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());
				ContextHandle.SetAbility(OwnerASC->GetAnimatingAbility());
				ContextHandle.AddSourceObject(this);
				SpecHandle = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), -Damage);
				
			}
		}

		//Use Monster
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
							if (UPawnMovementComponent* Movement = OwnerPawn->FindComponentByClass<UPawnMovementComponent>())
							{
								Movement->SetPlaneConstraintEnabled(false);
							}
						}
					}
				}
				else
				{
					//DetectTarget 이 없으면 앞으로 나가기
					FVector ForwardVec = MeshComp->GetOwner()->GetActorForwardVector();
					LaunchTarget(MeshComp, ForwardVec, true, true);
				}
			}
		}
		//Use Player Or Other
		else
		{
			FVector ForwardVec = MeshComp->GetOwner()->GetActorForwardVector();
			FVector LaunchVelocity = ForwardVec * LaunchForce;
			LaunchTarget(MeshComp, LaunchVelocity, true, true);
		}
	}
	
}

void UNAAnimNotifyState_QuickDash::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	EventReference.GetNotify();


	// 에디터에서 서버도 같이 찾아가지고 gameworld 먼저 확인
	if (MeshComp->GetWorld()->IsGameWorld())
	{

		// 일단 몬스터만 사용 가능하게
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
							// 추적시 
							if (Tracing)
							{
								FVector DetectedPlayerLocation = DetectedPlayerActor->GetActorLocation();
								FVector OwnerLocation = MeshComp->GetOwner()->GetActorLocation();
								FVector LaunchDirection = (DetectedPlayerLocation - OwnerLocation).GetSafeNormal(); // 목표 방향 벡터
								FVector LaunchVelocity = LaunchDirection * LaunchForce;
								LaunchTarget(MeshComp, LaunchVelocity, true, true);
							}
							//추적 안할 경우
							else
							{
								FVector ForwardVec = MeshComp->GetOwner()->GetActorForwardVector();
								FVector LaunchVelocity = ForwardVec * LaunchForce;
								LaunchTarget(MeshComp, LaunchVelocity, true, true);
							}

						}
					}
				}
			}
		}
		// 서버 충돌
		if (MeshComp->GetOwner()->HasAuthority())
		{
			if (FrameDeltaTime)
			{
				const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
				TArray<FOverlapResult> OverlapResults;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(MeshComp->GetOwner()); // 시전자 제외
				const bool bOverlap = MeshComp->GetWorld()->OverlapMultiByChannel
				(
					OverlapResults,
					SocketLocation,
					FQuat::Identity,
					ECC_Pawn,
					FCollisionShape::MakeSphere(SphereRadius),
					QueryParams
				);
				if (!OverlapResults.IsEmpty() && MeshComp->GetWorld()->IsGameWorld())
				{
					const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

					if (!SourceInterface)
					{
						// GAS가 없는 객체로부터 시도됨
						check(false);
						return;
					}
					for (const FOverlapResult& OverlapResult : OverlapResults)
					{
						if (const TScriptInterface<IAbilitySystemInterface>& TargetInterface = OverlapResult.GetActor())
						{
							if (!AppliedActors.Contains(OverlapResult.GetActor()))
							{

								SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
								(
									*SpecHandle.Data.Get(),
									TargetInterface->GetAbilitySystemComponent()
								);
								// player 확인후 날리기
								if (ANACharacter* Player = Cast<ANACharacter>(OverlapResult.GetActor()))
								{
									FVector ForwardVec = MeshComp->GetOwner()->GetActorForwardVector();
									FVector LaunchVelocity = ForwardVec * LaunchForce;
									LaunchVelocity.Z = 1200;
									Player->LaunchCharacter(LaunchVelocity, true, true);
								}
								AppliedActors.Add(OverlapResult.GetActor());
							}
						}
					}
				}
			}
		}
	}
	else
	{
#if WITH_EDITOR || UE_BUILD_DEBUG
		const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MeshComp->GetOwner()); // 시전자 제외
		const bool bOverlap = MeshComp->GetWorld()->OverlapMultiByChannel
		(
			OverlapResults,
			SocketLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(SphereRadius),
			QueryParams
		);
		DrawDebugSphere
		(
			MeshComp->GetWorld(),
			SocketLocation,
			SphereRadius,
			8,
			bOverlap || !OverlapResults.IsEmpty() ? FColor::Green : FColor::Red
		);
#endif 
	}
}

void UNAAnimNotifyState_QuickDash::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{

	// 에디터에서 서버도 같이 찾아가지고 gameworld 먼저 확인
	if (MeshComp->GetWorld()->IsGameWorld())
	{
		SpecHandle.Clear();
		ContextHandle.Clear();
		AppliedActors.Empty();

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
							if (UPawnMovementComponent* Movement = OwnerPawn->FindComponentByClass<UPawnMovementComponent>())
							{
								Movement->SetPlaneConstraintEnabled(true);
							}
						}
					}
				}

			}
		}



	}
}

void UNAAnimNotifyState_QuickDash::LaunchTarget(USkeletalMeshComponent* MeshComp, FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	if (APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (UPawnMovementComponent* Movement = OwnerPawn->FindComponentByClass<UPawnMovementComponent>())
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
