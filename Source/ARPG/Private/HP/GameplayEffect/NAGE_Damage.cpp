// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Damage.h"

#include "Ability/AttributeSet/NAAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UNAGE_Damage::UNAGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	/* 모디파이어가 적용되기 전에 수행할 작업
	FGameplayEffectExecutionDefinition ExecutionDefinition;
	ExecutionDefinition.CalculationClass = UNAHealthExecutionCalculation::StaticClass();

	FGameplayEffectExecutionScopedModifierInfo ScopedModifer;
	FGameplayEffectAttributeCaptureDefinition AttributeCapture;
	AttributeCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AttributeCapture.AttributeToCapture = UNAAttributeSet::GetHealthAttribute();
	ScopedModifer.CapturedAttribute = AttributeCapture;
	ExecutionDefinition.CalculationModifiers.Add( ScopedModifer );
	Executions.Add( ExecutionDefinition );
	*/

	AssetTagsComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>("TargetTagsGameplayEffectComponent");
	GEComponents.AddUnique( AssetTagsComponent );

	FInheritedTagContainer Container;
	Container.AddTag( FGameplayTag::RequestGameplayTag( TEXT( "Data.Health") ) );
	AssetTagsComponent->SetAndApplyAssetTagChanges( Container );

	FGameplayModifierInfo Modifier;
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.Attribute = UNAAttributeSet::GetHealthAttribute();
	
	FSetByCallerFloat CallerValue;
	CallerValue.DataTag = FGameplayTag::RequestGameplayTag( TEXT( "Data.Damage" ) );
	Modifier.ModifierMagnitude = CallerValue;

	Modifiers.Add( Modifier );
}
