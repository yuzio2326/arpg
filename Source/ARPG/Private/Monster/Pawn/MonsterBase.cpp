// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/Pawn/MonsterBase.h"
#include "Net/UnrealNetwork.h"

//Timer
#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"

//Ability
//#include "Ability/GameplayAbility/AttackGameplayAbility.h"
#include "Monster/Ability/GameplayAbility/GA_MonsterAttack.h"
#include "Monster/Ability/GameplayAbility/GA_Spawning.h"
#include "Monster/Ability/GameplayAbility/GA_UseSkill.h"
#include "Monster/Ability/GameplayAbility/NAGA_Death.h"
#include "Ability/GameplayAbility/NAGA_Suplexed.h"
#include "GameplayEffectExtension.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Monster/DataTable/MonsterOwnTableRow.h"
#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"

//DEFINE_LOG_CATEGORY(LogTemplateMonster);

// Sets default values
AMonsterBase::AMonsterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionObjectType( ECC_Pawn );

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*TODO:: Not Use right now*/

	/*TODO:: After Create MovementComponents or Change MovementComponents 
	if someone Use UFloatingPawnMovement Delete this Comment And Used it plz or someone do not need this delete all this codes*/
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	check(MovementComponent);
	/*Use this Afeter Create StateComponent*/
	//StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	//check(StatusComponent);
	
	/*Use this Afeter Create SkillComponent(Finishkill) */
	//SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("MonsterSkillComponent"));
	//check(SkillComponent);

	
	/*AI*/
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	AISenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISenseConfig_Sight"));
	AISenseConfig_Sight->DetectionByAffiliation.bDetectNeutrals = true;
	AISenseConfig_Sight->SightRadius = 800.f;
	AISenseConfig_Sight->LoseSightRadius = 1000.f;
	AISenseConfig_Sight->PeripheralVisionAngleDegrees = 120.f;
	AIPerceptionComponent->ConfigureSense(*AISenseConfig_Sight);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	


	//DefaultCombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>(TEXT("DefaultCombatComponent"));

	AutoPossessAI = EAutoPossessAI::Spawned;
}

void AMonsterBase::InitializeAbilities()
{
}

void AMonsterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			// 플레이어 스테이트 없이 몬스터가 처리
			AbilitySystemComponent->InitAbilityActorInfo(this, this);

			MovementComponent->SetPlaneConstraintEnabled(true);
			MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 1));
			MovementComponent->SetPlaneConstraintOrigin(GetOwner()->GetActorLocation());
		}

		SetOwner(NewController);

		SetAttributeData(OwnStatData);
	}




}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	//TSubclassOf<AAIController> MainAIControllerClass = AMonsterAIController::StaticClass();
	//AIControllerClass = MainAIControllerClass;

	if (HasAuthority())
	{
		if (AbilitySystemComponent) 
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_MonsterAttack::StaticClass(), 1, 0));
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_Spawning::StaticClass(), 1, 0));
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_UseSkill::StaticClass(), 1, 0));
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UNAGA_Death::StaticClass(), 1, 0));
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UNAGA_Suplexed::StaticClass(), 1, 0));
		}

	}

}

void AMonsterBase::SetAttributeData(const FDataTableRowHandle& InDataTableRowHandle)
{		
	
	if (FMonsterOwnTable* Data = InDataTableRowHandle.GetRow<FMonsterOwnTable>(TEXT("MonsterStatData")))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UNAAttributeSet::GetHealthAttribute()).AddUObject(this, &AMonsterBase::initializeAttribute);

		// Damage 관련은 UNAAttributeSet애 없음으로 새로 만들거나 여기에 만들어 놓은 BaseDamage를 GAS 가 가져갈수 있도록 처리할 필요가 있음
		AbilitySystemComponent->SetNumericAttributeBase(UNAAttributeSet::GetMaxHealthAttribute(), Data->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UNAAttributeSet::GetHealthAttribute(), Data->Health);
		AbilitySystemComponent->SetNumericAttributeBase(UNAAttributeSet::GetMovementSpeedAttribute(), Data->MovementSpeed);
		MovementComponent->MaxSpeed = Data->MovementSpeed;

		BaseDamage = Data->BaseDamaage;
	}
	// Failed
	else
	{
		//DataTable을 만들어 주세요
		Data = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Please Create Monster Stat DataTable"));
	}


}

void AMonsterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMonsterBase, AbilitySystemComponent);
}
#pragma region Legacy
//Gas 전환 완료
//bool AMonsterBase::OnAttack()
//{
//	//AIControllerClass에서 OnAttack하도록 호출하고 AIControllerClass에서 가지고있는 component를 가져와 공격하도록 해야하나?
//	AIControllerClass;
//
//	return false;
//}
#pragma endregion

//float AMonsterBase::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
//{
//
//	if (Controller)
//	{
//		//damage 맞을때 멈칫하려고 하지 않는다면 해당 부분은 delete해주세요
//		Controller->StopMovement();
//	}
//
//	return 0.0f;
//}

void AMonsterBase::TakeDamageStun(float HP, UAnimMontage* TakeDamageMontage, float StunSpeed)
{
	if (HasAuthority())
	{

	}
}

void AMonsterBase::OnDie()
{
	// Check
	if (HasAuthority())
	{
		if (DeathMontage || SuplexedMontage)
		{
			UAnimMontage* CurrentMontage = AbilitySystemComponent->GetCurrentMontage();
			if (AbilitySystemComponent->GetCurrentMontage() == DeathMontage || AbilitySystemComponent->GetCurrentMontage() == SuplexedMontage)
			{
				UAnimMontage* DeathMontageCheck = AbilitySystemComponent->GetCurrentMontage();
				float CheckLeftTime = AbilitySystemComponent->GetCurrentMontageSectionTimeLeft();
				if (CheckLeftTime < 0.3f)
				{
					DropItem(OwnStatData);
					//BodySlash();
					Destroy();
				}
				//bool bForcheck = true;
			}
		}
		else
		{
			//BodySlash();
			Destroy();
			
		}
	}
}
void AMonsterBase::BodySlash()
{
	//const USkeletalMeshSocket* RootSocket = SkeletalMeshComponent->GetSocketByName(SkeletalMeshComponent->GetBoneName(0));
	//const int32 NumBones = SkeletalMeshComponent->GetNumBones();


	//SkeletalMeshComponent->SetIsReplicated(true);
	//SkeletalMeshComponent->SetSimulatePhysics(true);
	//SkeletalMeshComponent->AddImpulseToAllBodiesBelow(FVector(0, 0, 200), NAME_None, true);
	//SkeletalMeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));

	// 오체분시
	//SkeletalMeshComponent->HideBoneByName(NAME_None, EPhysBodyOp::PBO_Term);
	

	// 실패
	if (HasAuthority())
	{

		// physicsAsset 에 있는 뼈 이름 가지고 와서 해당 뼈들만 hideBonebyName 으로 제거한뒤 impulse 로 날려보내면 비슷하지 않을까?
		//const UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
		/* 임시 폭발 */
		//SkeletalMeshComponent->SetIsReplicated(true);
		//SkeletalMeshComponent->SetSimulatePhysics(true);
		//SkeletalMeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
		//SkeletalMeshComponent->HideBoneByName(NAME_None, EPhysBodyOp::PBO_Term);
		//SkeletalMeshComponent->AddImpulseToAllBodiesBelow(FVector(0, 0, 1000), NAME_None, true);

		// 이러면 몸통 하나만 남고 나머지는 사라짐..
		// 그렇다면 반대로 몸통 하나만 사라지게 하는 방법은?
		// 얘는 일단 몸통 하나만이고..
		//for (const USkeletalBodySetup* SlashBodySetup : PhysicsAsset->SkeletalBodySetups)
		//{
		//	//Spine03 만 남음
		//	if (SlashBodySetup)
		//	{
		//		const FName BoneName = SlashBodySetup->BoneName;
		//		SkeletalMeshComponent->BreakConstraint(FVector::ZeroVector, FVector::ZeroVector, BoneName);
		//		SkeletalMeshComponent->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
		//	}
		//	// Spine03 빼고 남겨야됌...
		//	if (SlashBodySetup)
		//	{
		//		const FName BoneName = SlashBodySetup->BoneName;
		//		// Root 본만 골라서 처리
		//		if (BoneName == SkeletalMeshComponent->GetBoneName(0)) // 또는 "pelvis", "root" 등 명시적 이름
		//		{
		//			SkeletalMeshComponent->BreakConstraint(FVector::ZeroVector, FVector::ZeroVector, BoneName);
		//			SkeletalMeshComponent->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
		//		}
		//	}
		//}




		FTimerHandle SplashHandle;
		GetWorld()->GetTimerManager().SetTimer(SplashHandle, FTimerDelegate::CreateLambda([this]()
			{
				Destroy();
			}), 3.0f, false);

	}


}

void AMonsterBase::DropItem(const FDataTableRowHandle& InDataTableRowHandle)
{
	// 서버에서 드랍
	if (HasAuthority())
	{
		// datatable 가져오고
		if (FMonsterOwnTable* Data = InDataTableRowHandle.GetRow<FMonsterOwnTable>(TEXT("MonsterStatData")))
		{
			// 각 아이템 마다 확률로 드랍
			for (const FNADropItemPair Item: Data->ItemClass)
			{
				if (Item.ItemClasses)
				{
					float RandomDrop = FMath::RandRange(0.0f,1.0f);
					// 드랍하는곳
					if (RandomDrop < Item.Probability)
					{

						const FVector& SpawnLocation = GetActorLocation() + FVector(FMath::RandRange(-80,80), FMath::RandRange(-80, 80), FMath::RandRange(150, 300));
						const FRotator& SpawnRotation = GetActorRotation();
						AActor* SpawnedItem = GetWorld()->SpawnActor(Item.ItemClasses, &SpawnLocation, &SpawnRotation);
						SpawnedItem->SetReplicates(true);						
					}
				}
				else
				{
					// 향상된 for 문 사용하면 몇번째인지 알수는 없네... 그냥 옛날 for문 돌릴까?
					UE_LOG(LogTemp, Log, TEXT("MonsterStat Item is Null please Check the Item"));
				}
			}
		}

	}
}

void AMonsterBase::initializeAttribute(const FOnAttributeChangeData& Data)
{
	// 처음으로 체력이 0 이하가 될때
	if (Data.NewValue <= 0.f && Data.OldValue > 0.f)
	{
		OnHealthDepleted();
	}
	float m_fHealth = Cast<UNAAttributeSet>(AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();
	float m_fMaxHealth = Cast<UNAAttributeSet>(AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetMaxHealth();
	float HealthRatio = m_fHealth / m_fMaxHealth;
	// 보스몹은 일정 체력 이하일때만 suplex 사용 가능하도록 처리
	if (m_fHealth<=100)
	{
		//suplex 가능하도록 여기에 추가
	}



}

void AMonsterBase::OnHealthDepleted()
{
	// 서버에서 작업
	if (HasAuthority())
	{
		// 사망 처리
		OnDie();
	}
}

//ASC에서 GameplayEffect를 적용받을 때 호출
void AMonsterBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// 충돌을 할때마다 hpcheck를 해
	// hp가 떨어졌으면 player 를 찾아 


	AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass());
	Cast<UNAAttributeSet>(AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();
	auto a=0;

	const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()));
	if (AttributeSet)
	{
		FGameplayAttribute HealthAttribute = UNAAttributeSet::GetHealthAttribute();

		if (Data.EvaluatedData.Attribute == HealthAttribute)
		{
			const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
			AActor* InstigatorActor = EffectContext.GetInstigator();

		}
	}
	

}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (AbilitySystemComponent)
	{
		float m_fHealth = Cast<UNAAttributeSet>(AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();
		if (m_fHealth <= 0)
		{
			OnHealthDepleted();
		}
		else
		{
			TakeDamageStun(m_fHealth, DamageMontage,1.f);
		}

		bool check = false;
	}


}


