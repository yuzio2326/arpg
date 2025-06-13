// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/BTTask/BTTaskNode_Death.h"
#include "Monster/AI/MonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "Monster/Pawn/MonsterBase.h"
#include "Monster/Ability/GameplayAbility/NAGA_Death.h"

UBTTaskNode_Death::UBTTaskNode_Death()
{
	NodeName = TEXT("GAS Death");
    DeathAbilityClass = UNAGA_Death::StaticClass();

}

EBTNodeResult::Type UBTTaskNode_Death::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    AMonsterBase* Monster = Cast<AMonsterBase>(AIPawn);

    UAbilitySystemComponent* AbilitySystemComponent = AIPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!AbilitySystemComponent) return EBTNodeResult::Failed;

    if (!DeathAbilityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Spawn: DeathAbilityClass is not set!"));
        return EBTNodeResult::Failed;
    }

    bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(DeathAbilityClass);


    return EBTNodeResult::Type();
}
