// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/GA_Spawning.h"
#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Monster/AI/MonsterAIController.h"

void UGA_Spawning::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(ActorInfo->AvatarActor.Get()))
		{
			UAnimMontage* MonsterSpawnMontage = MonsterBase->GetSpawnMontage();
			UAbilitySystemComponent* MonsterASC = MonsterBase->GetAbilitySystemComponent();

			// 들어오는거 확인 모두 보유중
			if (MonsterSpawnMontage && MonsterASC)
			{
				// 재생완료
				UAnimInstance* AnimInstance = MonsterASC->GetAvatarActor()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();

				AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());

				if (AnimInstance && MonsterAIController)
				{
					float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterSpawnMontage, 1.0f);
					MonsterAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("Spawning"), false);
				}

				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}
			// SpawnMontage 없을때 AI 버그 생기는거 방지
			else
			{
				AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());
				MonsterAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("Spawning"), false);
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}
			
		}
	}
}
