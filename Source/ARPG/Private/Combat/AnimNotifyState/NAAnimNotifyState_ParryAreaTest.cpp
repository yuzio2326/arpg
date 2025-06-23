// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_ParryAreaTest.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "NACharacter.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Monster/Pawn/MonsterBase.h"


void UNAAnimNotifyState_ParryAreaTest::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);



	AActor* OwnerActor = MeshComp->GetOwner();

//#if WITH_EDITOR
//	if (GIsEditor && OwnerActor && OwnerActor->GetWorld() != GWorld) { return; }
//#endif

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
			if (UAbilitySystemComponent* OwnerASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>())
			{
				if (AMonsterBase* Monster = Cast<AMonsterBase>(MeshComp->GetOwner()))
				{

					const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
					ContextHandle = OwnerASC->MakeEffectContext();
					ContextHandle.AddOrigin(SocketLocation);
					ContextHandle.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());
					ContextHandle.SetAbility(OwnerASC->GetAnimatingAbility());
					ContextHandle.AddSourceObject(this);
					SpecHandle = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
					//SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), -Damage);
				
					// 그냥 asc에 박을까? 
					// Monster->GetBaseDamage()
					float Test = Monster->GetBaseDamage();
					SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( "Data.Damage" ), -Test);
				}
			}
		}
		// 서버와 클라이언트 간 플레이어 컨트롤러 설정 동기화
		if (MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>())
		{
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->bUseControllerDesiredRotation = false;
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->StopMovementImmediately();
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->DisableMovement();
		}
		// monster는 aicontroller가 서버에서 만들어 지기 때문에 동기화 과정은 필요 없을거 같음
		//else if (MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>())
		//{
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->bUseControllerDesiredRotation = false;
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->StopMovementImmediately();
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->DisableMovement();
		//}

	}

}

void UNAAnimNotifyState_ParryAreaTest::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
			//패링 성공시 데미지를 입지 않고 stun상태가 됩니다
			if (SuccessParry)
			{
				AnimInstance->Montage_Stop(0.2f);
				AnimInstance->Montage_Play(StunMontage);
			}
			//패링 실패시 대상에게 데미지를 주도록 합시다
			//AppliedActors 에 set되어있는 액터중 NACharacter만 가지고 와서 데미지 ㄱㄱ
			else
			{
				for (AActor* CheckActor : AppliedActors)
				{
					if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
					{
						const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();
						
						UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
						float HP = Cast<UNAAttributeSet>(PlayerASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();
						FGameplayTag SuplexTag = FGameplayTag::RequestGameplayTag("Player.Status.Suplex");
						//PlayerASC로 이미 플레이어로 고정해서 몬스터끼리 데미지는 주지 않는다
						//suplex 중이 아닐때만 데미지를 입히도록 한다
						if (!PlayerASC->HasMatchingGameplayTag(SuplexTag))
						{
							SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
								(
									*SpecHandle.Data.Get(),
									PlayerASC
								);				
						}
						float HP2 = Cast<UNAAttributeSet>(PlayerASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();
						bool check = false;

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

void UNAAnimNotifyState_ParryAreaTest::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	// 에디터에서 서버도 같이 찾아가지고 gameworld 먼저 확인
	if (MeshComp->GetWorld()->IsGameWorld())
	{
		// 충돌 처리는 서버의 책임
		if (MeshComp->GetOwner()->HasAuthority())
		{
			// 충돌 확인 지연	-> 어차피 텀은 적은데 그냥 tick에서 돌리는게 맞지 않을까? 
			OverlapElapsed += FrameDeltaTime;
			//interval time 넘어가면 충돌 확인 계속 하도록 하는거
			if (OverlapElapsed >= OverlapInterval)
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
					//검사 목록에서 NACharacter 검출 -> 이후 근접 공격 어빌리티 사용을 하는지 확인후 
					for (AActor* CheckActor : AppliedActors)
					{
						//Player Cast로 Playcheck 
						if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
						{
							UAbilitySystemComponent* PlayerASC =  Player->GetAbilitySystemComponent();
							// 공격을 하는게 combatcomponent네? 얘를 가지고 와서 해야하나?
							UNAMontageCombatComponent* PlayerCombatComponent = Player->FindComponentByClass<UNAMontageCombatComponent>();
							float ParryAngle = FVector::DotProduct(Player->GetActorForwardVector(), MeshComp->GetOwner()->GetActorForwardVector());

							float Dist = FVector::Dist(Player->GetActorLocation(), MeshComp->GetOwner()->GetActorLocation());

							//	상대 공격에 맞지 않아도 공격한 상태면 패링이 되는 문제가 있음 -> 이건 어떻게 해야할까?...
							//	생대 mesh와 owner mesh의 각도를 구해서 가져온뒤에 일정 각도 이하로 설정 ㄱ
							// 30도 이하 + 공격중 + 대상과의 거리 230 이하
							if (Dist < 230 && PlayerCombatComponent->IsAttacking() && ParryAngle < -0.85)
							{ 
								SuccessParry = true; 
							}
							else
							{ 
								SuccessParry = false; 
							}

						}					
					}			
				}
				OverlapElapsed = 0.f;
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
