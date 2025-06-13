// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/NAGA_Death.h"

#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Skill/DataTable/SkillTableRow.h"


UNAGA_Death::UNAGA_Death()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UNAGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}

	if (AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(ActorInfo->AvatarActor.Get()))
	{
		UAnimMontage* MonsterDeathMontage = MonsterBase->GetDeathMontage();
		UAbilitySystemComponent* MonsterASC = MonsterBase->GetAbilitySystemComponent();

		// 들어오는거 확인 모두 보유중
		if (MonsterDeathMontage && MonsterASC)
		{
			// 재생
			UAnimInstance* AnimInstance = MonsterASC->GetAvatarActor()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();

			AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());

			if (AnimInstance && MonsterAIController)
			{
				float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterDeathMontage, 1.0f);

				//float CurrentTime = AnimInstance->Montage_GetPosition(MonsterDeathMontage);
				//float MontageLength = MonsterDeathMontage->GetPlayLength();
				//float Progress = (CurrentTime / MontageLength) * 100.0f; // 진행 퍼센트 계산

				//if (Progress >= 90.0f)
				//{
				//	AnimInstance->Montage_Pause();
				//	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
				//	MonsterAIController->GetOwner()->Destroy();
				//}					
			}

		}
		// spawn Montage 가 없을 경우 바로 ai작동
		else
		{
			AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			MonsterAIController->GetOwner()->Destroy();
		}
	}
}
