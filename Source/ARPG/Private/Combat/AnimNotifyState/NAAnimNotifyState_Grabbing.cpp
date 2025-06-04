// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_Grabbing.h"

#include "Engine/OverlapResult.h"
#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "GameFramework/CharacterMovementComponent.h"




void UNAAnimNotifyState_Grabbing::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	// 이미 플레이어를 붙잡고 있는 형태 입니다

	AActor* OwnerActor = MeshComp->GetOwner();

#if WITH_EDITOR
	if (GIsEditor && OwnerActor && OwnerActor->GetWorld() != GWorld) { return; }
#endif

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		if (MeshComp->GetOwner()->HasAuthority())
		{
			if (UAbilitySystemComponent* OwnerASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>())
			{
				const FVector SocketLocation = MeshComp->GetSocketLocation(GrabSocketName);
				ContextHandle = OwnerASC->MakeEffectContext();
				ContextHandle.AddOrigin(SocketLocation);
				ContextHandle.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());
				ContextHandle.SetAbility(OwnerASC->GetAnimatingAbility());
				ContextHandle.AddSourceObject(this);
				SpecHandle = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
			}
		}
		// 서버와 클라이언트 간 플레이어 컨트롤러 설정 동기화
		if (MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>())
		{
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->bUseControllerDesiredRotation = false;
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->StopMovementImmediately();
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->DisableMovement();
		}

	}
}

void UNAAnimNotifyState_Grabbing::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	// 여기서 잡아두고 있는 대상 외에 다른 대상을 계속 찾고 해당 player가 공격하는 방향에 따라서 반응을 다르게 한다
	// 정면이면 바로 밀려 나도록 하고 후면이면 잡히고 있는 대상이 발로 차도록 해서 밀어내도록 한다
	// 그외 (잡힌 플레이어가 일정 수 만큼 공격을 시도하면 풀려나도록?)

	// 충돌 처리는 서버의 책임
	if (MeshComp->GetOwner()->HasAuthority())
	{
		// 충돌 확인 지연	-> 어차피 텀은 적은데 그냥 tick에서 돌리는게 맞지 않을까? 
		if (FrameDeltaTime)
		{
			// 일정 범위 내에 있는 모든 플레이어 조회
			const FVector SocketLocation = MeshComp->GetSocketLocation(MainSocketName);
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

#if WITH_EDITOR || UE_BUILD_DEBUG
			DrawDebugSphere
			(
				MeshComp->GetWorld(),
				SocketLocation,
				SphereRadius,
				8,
				bOverlap || !OverlapResults.IsEmpty() ? FColor::Green : FColor::Red
			);
#endif
			//충돌된게 게임월드일 경우
			if (!OverlapResults.IsEmpty() && MeshComp->GetWorld()->IsGameWorld())
			{
				const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

				if (!SourceInterface)
				{
					// GAS가 없는 객체로부터 시도됨
					check(false);
					return;
				}

				/*다른 player가 공격을 해서 풀려날때*/
				//AppliedActors 에 새로운 대상이 추가되면 검사 목록에 추가
				for (const FOverlapResult& OverlapResult : OverlapResults)
				{
					if (const TScriptInterface<IAbilitySystemInterface>& TargetInterface = OverlapResult.GetActor())
					{
						if (!AppliedActors.Contains(OverlapResult.GetActor()))
						{
							UE_LOG(LogTemp, Log, TEXT("[%hs]: Found target %s"), __FUNCTION__, *OverlapResult.GetActor()->GetName());
							//AppiedActor
							AppliedActors.Add(OverlapResult.GetActor());
						}
					}
				}
				// 검사 목록에서 NACharacter 검출-> 
				for (AActor* CheckActor : AppliedActors)
				{
					//Player Cast로 Playcheck 
					if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
					{
						UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
						// 공격을 하는게 combatcomponent네? 얘를 가지고 와서 해야하나?
						UNAMontageCombatComponent* PlayerCombatComponent = Player->FindComponentByClass<UNAMontageCombatComponent>();											
						// 정면에서 아군이 공격할때 날라가는 montage 사용
						if (PlayerCombatComponent->IsAttacking()) { SuccessEscape = true; }
						//후면에서 아군이 공격할때 쓰러지는 montage 사용
						else { SuccessEscape = false; }
					}
				}
				/* player 혼자서 풀려날때 */


			}
		}
	}
}

void UNAAnimNotifyState_Grabbing::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	// 만들고 나서 보니까 이거 잘하면 잡기 판정도 만들수 있을거 같은데?...
	// 심지어 대상을 내 소켓애다가 박아놓고 tick동안 움직임 고정 시키고 end 때 풀어놓으면?
	// tick동안 대상에게 특정 montage 강제 시켜놓으면 잡기도 될거 같은데????
	// ParryArea 같이 쓰면 잡기 시전중에 패링시키고 실패시 잡혀가는것도 될거 같은데?????

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		/* 공격 판정 apply */
		if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
		{
			UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance();
			//탈출 성공시 데미지를 입히지 않고 날라가도록
			if (SuccessEscape)
			{
				AnimInstance->Montage_Stop(0.2f);
				AnimInstance->Montage_Play(EscapeMontage);
				for (AActor* CheckActor : GrabbingActors)
				{
					if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
					{
						const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();
						UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
						UAnimInstance* PlayerAnimInstance = PlayerASC->AbilityActorInfo->GetAnimInstance();
						PlayerAnimInstance->Montage_Stop(0.2f);
						PlayerAnimInstance->Montage_Play(PlayerEscapeMontage);
					}
				}
			}
			else
			{
				AnimInstance->Montage_Stop(0.2f);
				AnimInstance->Montage_Play(EscapeFailedMontage);
				for (AActor* CheckActor : GrabbingActors)
				{
					if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
					{
						const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();
						UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
						UAnimInstance* PlayerAnimInstance = PlayerASC->AbilityActorInfo->GetAnimInstance();
						AnimInstance->Montage_Stop(0.2f);
						AnimInstance->Montage_Play(PlayerEscapeFailedMontage);
						SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
						(
							*SpecHandle.Data.Get(),
							PlayerASC
						);
					}
				}
			}
		}
		//재사용성을 위해 clear 및 empty
		if (MeshComp->GetOwner()->HasAuthority())
		{
			SpecHandle.Clear();
			ContextHandle.Clear();
			AppliedActors.Empty();
		}

	}
}
