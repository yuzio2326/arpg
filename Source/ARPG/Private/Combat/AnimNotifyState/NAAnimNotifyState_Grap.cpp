// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_Grap.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "NACharacter.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Ability/GameplayAbility/NAGA_Grab.h"

void UNAAnimNotifyState_Grap::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Player의 잡기와 monster의 잡기를 구분..
	 
#pragma region 주석
	// Monster가 사용하면 잡기 시전 애니메이션이 먼저 나가고 해당 범위 내에 플레이어가
	// 존재할 경우 플레이어도 잡기 저항(잡기 도중) 모션이 나가도록 하고
	// 이후 일정 스택 이상 공격 성공시 또는 다른 플레이어가 근접공격 성공시 떨어져 나가도록 하기

	// state는 grabbing 으로 하고 주변 플레이어 위치를 받고 
	
	// 플레이어가 사용할 경우 
	// null offset에 monster의 root고정 시켜야 됌		//bone 보니까 이건 세팅 안한거 같음.. 일정 위치로 해야 할거 같음
	// 상대 몬스터의 회전 확인(뒤쪽일것) -> 가능하도록 하기 ㄱㄱ
	// 무기에 따른 공격이니까 idle에서 해당 notify를 불러내서 사용하면 되려나?
	// 권총, ㄴ무기 = splex	//	knife =  knifeAction으로

	// idle에서 호출했다고 치고 그러면 begin 할때 구체 만들어서 몬스터의 rot 가져오고 위치는 구체 하나 박아 놓고 
	// 해당 구체에 닿으면 추가적인 ui를 보이게 한다거나 tick에서 보여주게 하고 tick중에 해당 트리거 on하면 바로 애니메이션 몽타주 재생
#pragma endregion

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
				const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
				ContextHandle = OwnerASC->MakeEffectContext();
				ContextHandle.AddOrigin(SocketLocation);
				ContextHandle.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());
				ContextHandle.SetAbility(OwnerASC->GetAnimatingAbility());
				ContextHandle.AddSourceObject(this);
				SpecHandle = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
				if ( const UNAMontageCombatComponent* CombatComponent = MeshComp->GetOwner()->GetComponentByClass<UNAMontageCombatComponent>() )
				{
					SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( "Data.Damage" ), -CombatComponent->GetBaseDamage() );
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

	}
}

void UNAAnimNotifyState_Grap::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{

	Super::NotifyEnd(MeshComp, Animation, EventReference);

#pragma region 주석
	// Monster 후방에 있어야 함 + 해당 몬스터가 잡히는 모션이 있어야함 -> monster에서 작동하도록 할까?
	// player 한테 전달해서 모션 강제로 전환도 가능함
	// Monster가 idle일때 사용가능하도록 하면 ㄱㅊ지 않을까?
	// Monster가 사용하는 잡기는?
	// Monster가 사용-> 시전 동작-> 닿으면 대상 잡히는 모션 전환-> 중간에 탈출 트리거 만들어 놓기
	// Player가 사용-> Monster 후방에 있어야 함-> 플레이어가 트리거 작동-> 몬스터와 플레이어 잡히는 모션 전환
	
	// 몬스터마다 가지고있는 모션은 다 다르니까 idle상태에서 notify를 만들고 
	// 플레이어가 후방에서 공격 시전시 모션을 캔슬하고 잡는걸로 바꾸는게 ㄱㅊ을듯?
	
	// 몬스터가 잡는걸 시전 한다고 치면 잡는 모션에 이 notify를 박고 잡는지 확인-> 대상에 플레이어가 있으면 잡기 없으면 idle로 돌아가기
	// 플레이어가 잡는걸 시전 한다고 치면 후방에 있어야 하고 monster의 idle에 해당 notify를 박고 플레이어 확인-> 플레이어 공격 시전시
	// 플레이어 공격 캔슬 -> 잡기 모션 전환 + 몬스터 잡히는 모션 전환 이후 즉사 or damage 
	
	// Player의 공격을 가지고 오고 Monster의 현재 상태에서 작동하도록 하는걸로 하면  굳이 gameplayability 를 만들 필요가 없을거 같음
	// 하지만 몬스터의 Grab은 이걸로 작동X Skill로 만들어서 박아 놓으면 작동하고 notify로 잡힌 상태나 다른걸로 전환 시키면 될듯?


	// 만들고 나서 보니까 이거 잘하면 잡기 판정도 만들수 있을거 같은데?...
	// 심지어 대상을 내 소켓애다가 박아놓고 tick동안 움직임 고정 시키고 end 때 풀어놓으면?
	// tick동안 대상에게 특정 montage 강제 시켜놓으면 잡기도 될거 같은데????
	// ParryArea 같이 쓰면 잡기 시전중에 패링시키고 실패시 잡혀가는것도 될거 같은데?????
#pragma endregion

	//
	if (MeshComp->GetWorld()->IsGameWorld())
	{
		for (AActor* CheckActor : AppliedActors)
		{
			//Player Cast로 Playcheck 
			if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
			{
				UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
				const FVector OffsetLocation = MeshComp->GetSocketLocation(OffsetName);
				Player->SetActorLocation(OffsetLocation);

				// 임시로 만든 것 입니다 grabbing NAGA 만들고 넣어야 함
				FGameplayAbilitySpec* AbilitySpec = PlayerASC->FindAbilitySpecFromClass(UNAGA_Grab::StaticClass());
				UNAGA_Grab* NAGA_Grab = Cast<UNAGA_Grab>(AbilitySpec->Ability);
				PlayerASC->PlayMontage(NAGA_Grab,AbilitySpec->ActivationInfo, GrabedMontage,1);
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

void UNAAnimNotifyState_Grap::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	//GameWorld 일때 
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

				//충돌되었을때
				if (!OverlapResults.IsEmpty())
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

					// 검사 목록에서 NACharacter 검출-> 
					for (AActor* CheckActor : AppliedActors)
					{
						//Player Cast로 Playcheck 
						if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
						{
							UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
							// 공격을 하는게 combatcomponent네? 얘를 가지고 와서 해야하나?
							UNAMontageCombatComponent* PlayerCombatComponent = Player->FindComponentByClass<UNAMontageCombatComponent>();
							float GrapAngle = FVector::DotProduct(Player->GetActorForwardVector(), MeshComp->GetOwner()->GetActorForwardVector());

							// 플레이어가 공격중이 아닐경우 잡기 성공	parry도 같이넣어서 공격중이면 패링 ㄱ
							if (!PlayerCombatComponent->IsAttacking()) { SuccessGrab = true; }
							else { SuccessGrab = false; }

						}
					}
				}
				OverlapElapsed = 0.f;
			}
		}
	}
}
