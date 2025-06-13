// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_Death.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_Death : public UGameplayAbility
{
	GENERATED_BODY()
	
	UNAGA_Death();
public:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* AnimMontage;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	
	
};
