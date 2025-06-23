// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Suplex.h"

#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_Suplex::UNAGE_Suplex()
{
	TargetTagsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>("TargetTagsGameplayEffectComponent");
	GEComponents.AddUnique(TargetTagsGameplayEffectComponent);

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag("Player.Status.Suplex"));
	TargetTagsGameplayEffectComponent->SetAndApplyTargetTagChanges(TagContainer);

	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
