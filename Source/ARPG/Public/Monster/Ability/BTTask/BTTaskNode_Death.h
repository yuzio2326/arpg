// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_Death.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UBTTaskNode_Death : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_Death();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
protected:
	/* 호출할 어빌리티 클래스 */
	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<class UGameplayAbility> DeathAbilityClass;

	
};
