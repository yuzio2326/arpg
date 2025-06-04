// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Grab.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Monster/Pawn/MonsterBase.h"


UNAGA_Grab::UNAGA_Grab()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UNAGA_Grab::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 델레게이션을 지우고, 효과를 종료함 (가정: 어빌리티가 인스턴싱 되는 경우)
	const FGameplayAbilityActivationInfo Info = GetCurrentActivationInfo();

	// 추적하던 몽타주를 해제
	if (HasAuthority(&Info))
	{
		GetCurrentActorInfo()->AbilitySystemComponent->AbilityActorInfo->GetAnimInstance()->OnMontageEnded.RemoveAll(this);
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UNAGA_Grab::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 실행 가능 여부 확인은 서버로
	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}
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
	// 몬스터의 ASC입니다
	if (UAbilitySystemComponent* ASC = ActorInfo->AvatarActor.Get()->FindComponentByClass<UAbilitySystemComponent>())
	{		
		if (AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimMontage* SkillMontage = MonsterBase->GetSelectSkillMontage())
			{
				
			}
		}

	}


}

bool UNAGA_Grab::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	return false;
}

void UNAGA_Grab::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
}
