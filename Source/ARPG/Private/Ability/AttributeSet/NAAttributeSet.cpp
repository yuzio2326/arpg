// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/NAAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Net/UnrealNetwork.h"
#include "Monster/AI/MonsterAIController.h"

void UNAAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( UNAAttributeSet, Health );
	DOREPLIFETIME( UNAAttributeSet, MaxHealth );
	DOREPLIFETIME( UNAAttributeSet, AP );
	DOREPLIFETIME( UNAAttributeSet, MovementSpeed );
}

bool UNAAttributeSet::PreGameplayEffectExecute( struct FGameplayEffectModCallbackData& Data )
{
	const bool bResult = Super::PreGameplayEffectExecute( Data );

	if ( bResult )
	{
		if ( Data.EffectSpec.Def->IsA( UNAGE_Damage::StaticClass() ) )
		{
			if ( Data.EvaluatedData.Magnitude > 0)
			{
				Data.EvaluatedData.Magnitude = -Data.EvaluatedData.Magnitude;	
			}
		}
	}
	
	return bResult;
}

void UNAAttributeSet::PostGameplayEffectExecute( const struct FGameplayEffectModCallbackData& Data )
{
	if ( GetHealth() > GetMaxHealth() )
	{
		SetHealth( GetMaxHealth() );
	}


	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{

		const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
		UObject* Instigator = EffectContext.GetInstigator();
		if (Instigator)
		{
			if (AAIController* AIController = Cast<AAIController>(GetOwningActor()->GetInstigatorController()))
			{
				if (UBlackboardComponent* MonsterAiBB = AIController->GetBlackboardComponent())
				{
					MonsterAiBB->SetValueAsObject(TEXT("DetectPlayer"), Instigator);
					MonsterAiBB->SetValueAsObject(TEXT("LastDamageInstigator"), Instigator);
				}

			}
		}
	}


}
