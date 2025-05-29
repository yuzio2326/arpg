// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_ParryAreaTest.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

void UNAAnimNotifyState_ParryAreaTest::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor =  MeshComp->GetOwner();

	// For EditorPlay AnimSequence
#if WITH_EDITOR
	if (GIsEditor && OwnerActor && OwnerActor->GetWorld() != GWorld) { return; }
#endif

	//Test 용으로 begin되자마자 attribute에 데미지 넣고 패링 되는지 확인 ㄱㄱ
	if (UAbilitySystemComponent* OwnerASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>())
	{
		//const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(OwnerASC->GetAttributeSet(UNAAttributeSet::StaticClass()));
		//CheckHP = AttributeSet->GetHealth();

		// 데미지
		FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();


		EffectContext.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());

		// Gameplay Effect CDO, 레벨?, ASC에서 부여받은 Effect Context로 적용할 효과에 대한 설명을 생성
		const FGameplayEffectSpecHandle DamageEffectSpec = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1, EffectContext);

		// 설명에 따라 효과 부여 (본인에게)
		const auto& Handle = OwnerASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec.Data.Get());

		check(Handle.WasSuccessfullyApplied());


		bool checkOwnerASC = false;

	}

}

void UNAAnimNotifyState_ParryAreaTest::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	/*End에서 사용할경우*/
	if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>()) 
	{
		UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance();
		if (Check)
		{
			AnimInstance->Montage_Stop(0.2f);
			AnimInstance->Montage_Play(StunMontage);
		}

	}
}

void UNAAnimNotifyState_ParryAreaTest::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	//Owner의 Parry Area 에서 근접공격 맞은 애들은 parry 를 당함. 근접공격은 player쪽에서 넘겨가지고 attribute 를 조정한다고 하면
	// tick동안 attribute를 가지고 와서 근접 공격에 맞았으면 parry 아니면 계속 공격
	if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
	{
		UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance();

		OwnerASC->GetCurrentMontage();

		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(OwnerASC->GetAttributeSet(UNAAttributeSet::StaticClass()));
		//Take Damage is true -> Stopmontage change to Parry


		//OwnerASC->GetAllAttributes();
		// 
		//damage 맞았다고 대충 치고		++이때 패링 이펙트 파티클 같은거 보여줘서 패링 타이밍 보이게 하는것도 괜찮을거 같음
		Check = true;

		//패링 캔슬을 여기에 넣을까 END에 넣을까 고민이 좀 있긴함
		/**/
		if (Check)
		{
			AnimInstance->Montage_Stop(0.2f);
			AnimInstance->Montage_Play(StunMontage);
		}
	}



}
