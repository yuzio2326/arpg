// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerState.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Assets/Interface/NAManagedAsset.h"
#include "HP/ActorComponent/NAVitalCheckComponent.h"
#include "Net/UnrealNetwork.h"

float ANAPlayerState::GetHealth() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->GetHealth();
	}

	check(false);
	return 0.f;
}

int32 ANAPlayerState::GetMaxHealth() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->Health.GetBaseValue();
	}

	check(false);
	return 0.f;
}

bool ANAPlayerState::IsAlive() const
{
	// 체력 컴포넌트로부터 확인
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ) )
	{
		const ECharacterStatus CharacterStatus = Character->GetComponentByClass<UNAVitalCheckComponent>()->GetCharacterStatus();
		return CharacterStatus == ECharacterStatus::Alive || CharacterStatus == ECharacterStatus::KnockDown;
	}

	check( false );
	return false;
}

bool ANAPlayerState::IsKnockDown() const
{
	// 체력 컴포넌트로부터 확인
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ) )
	{
		const ECharacterStatus CharacterStatus = Character->GetComponentByClass<UNAVitalCheckComponent>()->GetCharacterStatus();
		return CharacterStatus == ECharacterStatus::KnockDown;
	}

	check( false );
	return false;
}

bool ANAPlayerState::IsZoom() const
{
	//
	//if (const ANACharacter* Character = Cast<ANACharacter>(GetPawn()))
	//{
	//	const ECharacterStatus CharacterStatus = Character->GetComponentByClass<UNAVitalCheckComponent>()->GetCharacterStatus();
	//	
	//}

	//check(false);
	return false;
}

void ANAPlayerState::SetPossessAssetName(const FName& AssetName)
{
	// 수정은 서버에서 가능하도록
	if ( HasAuthority() )
	{
		PossessAssetName = AssetName;
		UpdatePossessAssetByName();
	}
}

void ANAPlayerState::OnRep_PossessAssetName()
{
	UpdatePossessAssetByName();
}

void ANAPlayerState::UpdatePossessAssetByName()
{
	if ( const TScriptInterface<INAManagedAsset> Interface = GetPawn() )
	{
		Interface->SetAssetName( PossessAssetName );
	}

	// 중간에 에셋이 바뀐 경우이니, 다시 ASC를 업데이트
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetPawn() )
	{
		Interface->GetAbilitySystemComponent()->InitAbilityActorInfo( this, GetPawn() );
	}
}

void ANAPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void ANAPlayerState::PostNetInit()
{
	Super::PostNetInit();

	// 캐릭터는 기본 클래스를 주고 블루프린트로부터 복사해서 동적으로 적용
	// 캐릭터의 에셋이 중간에 바뀌어야 할 경우를 대비
	if (GetNetMode() == NM_Client)
	{
		TScriptDelegate Delegate;
		Delegate.BindUFunction(this, "UpdatePossessAssetByName");
		OnPawnSet.AddUnique(Delegate);
	}
}

void ANAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( ANAPlayerState, PossessAssetName )
}
