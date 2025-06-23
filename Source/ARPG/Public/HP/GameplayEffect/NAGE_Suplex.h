// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_Suplex.generated.h"

class UAssetTagsGameplayEffectComponent;
class UTargetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_Suplex : public UGameplayEffect
{
	GENERATED_BODY()
	
	UNAGE_Suplex();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UTargetTagsGameplayEffectComponent* TargetTagsGameplayEffectComponent;
	
};
