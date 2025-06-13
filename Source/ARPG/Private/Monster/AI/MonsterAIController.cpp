// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/AI/MonsterAIController.h"

#include "AbilitySystemComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"

#include "Monster/Pawn/MonsterBase.h"
#include "Ability/GameplayAbility/AttackGameplayAbility.h"
#include "Skill/DataTable/SkillTableRow.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Kismet/KismetSystemLibrary.h"

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();



}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// TODO:: there is No Component right Now Plz Add Components After Create monster Components


	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent())
		{
			bool OwnAbilitySystemComponent = true;
		}
	}

	/*AI Setting*/
	// BeginPlay는 Pawn이 만들어지기 전에 호출함으로 해당 위치로 호출 순서를 바꿨습니다
	if (!IsValid(BrainComponent))
	{
		//BT로 바꾸기
		UBehaviorTree* BehaviorTree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Script/AIModule.BehaviorTree'/Game/01_ExternalAssets/TempResource/Monster/AI/BT_BaseMonster.BT_BaseMonster'"));
		check(BehaviorTree);
		RunBehaviorTree(BehaviorTree);
		//Spawn 위치 기준 일정 범위 이상 못나가게 하려고 할때 사용 가능합니다
		APawn* OwningPawn = GetPawn();
		FVector FSpawnLocation = OwningPawn->GetActorLocation();
		Blackboard->SetValueAsVector(TEXT("SpwanPosition"), FSpawnLocation);
		Blackboard->SetValueAsBool(TEXT("Spawning"), true);
		

	}

}

void AMonsterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckHP();

	LookTarget();
	// 확인용
	Blackboard->SetValueAsBool(TEXT("LookPlayer"), LookOnTarget);

	//Montage play중이면 이동 멈추는거
	IsPlayingMontage();
	//Player 찾기
	FindPlayerByPerception();

	//Spawn 장소로 부터 일정 거리 떨어졌는지 확인하기
	CheckSpawnRadius();

	//Player와의 거리 확인
	CheckPlayerDistance();

	// 사거리 밖에 있으면 다시 random으로 뽑아서 사용하니 사거리 내에서 사용 가능한 스킬들을 random으로 사용
	if(!Blackboard->GetValueAsBool(TEXT("SelectedSkill")))
	{
		// 사용할 스킬 선택 -> 이후 ai가 사용하도록 ㄱㄱ
		SelectSkill();
	}
	else if (Blackboard->GetValueAsBool(TEXT("SelectedSkill")))
	{
		// Skill을 선택해서 세팅이 끝나서 사용이 가능함
		UseSkill();
	}
	
	changeSkillTime -= DeltaTime;
	

	// 로아식 ai 
	// 스킬을 선택하고 선택한 스킬의 사거리 까지 다가온뒤 스킬을 시전하는 형식 -> 체력이 일정량 이하로 떨어지면 기믹 패턴 수행
	
	//몬헌식 ai
	//player와의 거리를 가지고와 스킬들을 순회 하면서 사용 가능한 스킬의 사거리중에 하나를 고르고 사용해 // -> 돌진 스킬 필수네..
	// 밀라보레아스를 예시를 생각해 보면 첫 공격에는 무조건 돌진하는걸 보면 타겟이 멀리 있을때 돌진이 가능하면 돌진하고 연속으로 할때도 있으니 
	// stack형스킬로 가지고 있고 사용 가능하면 돌진 -> 돌진 불가능할 경우 브레스 종류를 쓰는거 같네
	// 


}

void AMonsterAIController::CheckSpawnRadius()
{
	FVector FSpawnLocation = Blackboard->GetValueAsVector(TEXT("SpwanPosition"));
	APawn* OwningPawn = GetPawn();
	FVector OwningPawnLocation = OwningPawn->GetActorLocation();
	//이동 반경
	float Radius = 20000;

	float Distance = FVector::Dist(FSpawnLocation, OwningPawnLocation);
	if (Distance > Radius) 
	{ 
		Blackboard->SetValueAsBool(TEXT("OutRangedSpawn"), true); 
		Blackboard->SetValueAsObject(TEXT("DetectTarget"), nullptr);
	}
	else { Blackboard->SetValueAsBool(TEXT("OutRangedSpawn"), false); }
}

void AMonsterAIController::CheckPlayerDistance()
{
	UObject* DetectedPlayer = Blackboard->GetValueAsObject(TEXT("DetectPlayer"));
	if (DetectedPlayer != nullptr)
	{
		if (AActor* DetectedPlayerActor = Cast<AActor>(DetectedPlayer))
		{
			APawn* OwningPawn = GetPawn();

			FVector OwningPawnLocation = OwningPawn->GetActorLocation();
			FVector DetectedPlayerLocation = DetectedPlayerActor->GetActorLocation();
	
			float Distance = FVector::Dist(DetectedPlayerLocation, OwningPawnLocation);

			Blackboard->SetValueAsFloat(TEXT("PlayerDistance"), Distance);
		}
	}

}

void AMonsterAIController::FindPlayerByPerception()
{
	APawn* OwningPawn = GetPawn();
	if (UAIPerceptionComponent* AIPerceptionComponent = OwningPawn->GetComponentByClass<UAIPerceptionComponent>())
	{
		TArray<AActor*> OutActors;
		AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISenseConfig_Sight::StaticClass(), OutActors);
		bool bFound = false;
		
		// 시야에 보인 Player를 찾는 부분입니다. -> 이후 선공을 하며 
		// 중간에 다른 사람이 공격할 경우 그 사람을 공격하려고 합니다
		for (AActor* It : OutActors)
		{
			if (ACharacter* DetectedPlayer = Cast<ACharacter>(It))
			{
				bFound = true;
				Blackboard->SetValueAsObject(TEXT("DetectPlayer"), Cast<UObject>(DetectedPlayer));
				break;
			}
		}

		bool m_bTakeDamage;

		if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
		{			
			UAbilitySystemComponent* MonsterASC = OwnerMonster->GetAbilitySystemComponent();
			float m_fHealth = Cast<UNAAttributeSet>(MonsterASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();			
			float m_fMaxHealth = Cast<UNAAttributeSet>(MonsterASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetMaxHealth();

			MonsterASC->GetAttributeSet(UNAAttributeSet::StaticClass());

			if (m_fHealth < m_fMaxHealth)
			{
				m_bTakeDamage = true;

				//Blackboard->SetValueAsObject(TEXT("DetectPlayer"), Cast<UObject>(DetectedPlayer));
			}


		}

		if (!bFound&& !m_bTakeDamage)
		{
			Blackboard->ClearValue(TEXT("DetectPlayer"));
		}
	}

}

void AMonsterAIController::FindDamageInstigator(AActor* DamageInstigatorTarget)
{



}

void AMonsterAIController::IsPlayingMontage()
{
	APawn* OwningPawn = GetPawn();

	// Montage가 Play 중이라면 BT 내부에서 AI 진행을 멈춘다
	const bool bMontagePlaying = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->IsAnyMontagePlaying();


	const UAnimMontage* CurrentMontage = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->GetCurrentActiveMontage();
	const float CurrentPosition = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->Montage_GetPosition(CurrentMontage);
	const float MontageLength = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->Montage_GetPlayRate(CurrentMontage);
	bool StopAI = false;
	//A가 사용중이면 Stop, A가 사용중인데 0.2보다 낮으면 Play가 되어야함
	if (MontageLength > 0.2f && bMontagePlaying) { StopAI = true; }
	else { StopAI = false; }

	Blackboard->SetValueAsBool(TEXT("MontagePlaying"), StopAI);


	if (!StopAI)
	{
		//Blackboard->SetValueAsBool(TEXT("UsingSkill"), false);
		Blackboard->SetValueAsBool(TEXT("OnDamage"), false);
	}

}
//임시 확인용 입니다
class UGA_UseSkill;
class UBTDecorator_TagCooldown;

void AMonsterAIController::SelectSkill()
{
	//사용할 스킬을 골라서 선택만 하도록 하고 CanUseSkill을 true로 ㄱㄱ
	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		UAbilitySystemComponent* MonsterASC = OwnerMonster->GetAbilitySystemComponent();

		FDataTableRowHandle OwnSkillData =OwnerMonster->GetSkillData();
		FOwnSkillTable* Data = OwnSkillData.GetRow<FOwnSkillTable>(TEXT("MonsterSkillData"));
		if (Data)
		{
			//보유한 스킬들
			//0번 1개 있을때 1이 됩니다.
			uint8 MonsterOwnskillNum = Data->OwnSkillArray.Num();

			//random 한번 돌리는 과정입니다
			// gas로 쿨타임 관리를 하고 사용을 하고
			if (MonsterOwnskillNum > 0)
			{
				// Array Index
				int64 Index = FMath::RandRange(0, MonsterOwnskillNum - 1);
				// 선택한 스킬을 몬스터로 보내 그걸 GAS가 잡아서 사용할거야
				OwnerMonster->SetSelectSkillMontage(Data->OwnSkillArray[Index].SkillMontage);

				// 선택한 스킬의 사거리는 blackboard에 세팅을 하고
				float SkillRange = Data->OwnSkillArray[Index].Range;

				// 이제 스킬이 세팅되서 사용이 가능해
				Blackboard->SetValueAsFloat(TEXT("SkillDistance"), SkillRange);			
				Blackboard->SetValueAsBool(TEXT("SelectedSkill"), true);
				changeSkillTime = 5.f;

			}
		}
	}
}

void AMonsterAIController::UseSkill()
{
	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{

		float SkillDistance = Blackboard->GetValueAsFloat(TEXT("SkillDistance"));
		float PlayerDistance = Blackboard->GetValueAsFloat(TEXT("PlayerDistance"));
		

		

		bool m_ChangeSkill = false;
		//skill 사거리 내에 들어왔을때 사용
		if (PlayerDistance < SkillDistance && LookOnTarget)
		{
			Blackboard->SetValueAsBool(TEXT("CanUseSkill"), true);

			// 위에 세팅된 몽타주를 재생하고 있으면 usingskill T 아니면 F
			if (OwnerMonster->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->Montage_IsPlaying(OwnerMonster->GetSelectSkillMontage()))
			{
				//재생하고 있으니까 CanUseSkill을 false
				Blackboard->SetValueAsBool(TEXT("UsingSkill"), true);
				Blackboard->SetValueAsBool(TEXT("CanUseSkill"), false);

				//스킬을 사용했음으로 false로 세팅
				Blackboard->SetValueAsBool(TEXT("SelectedSkill"), false);
			}
			else
			{ 
				Blackboard->SetValueAsBool(TEXT("UsingSkill"), false); 
			}

		}
		// 사거리 밖일때
		else if (PlayerDistance > SkillDistance)
		{
			Blackboard->SetValueAsBool(TEXT("CanUseSkill"), false);		
			if (changeSkillTime < 0)
			{
				SelectSkill();

				// 어그로 변경
				if (Blackboard->GetValueAsObject(TEXT("LastDamageInstigator")))
				{
					Blackboard->SetValueAsObject(TEXT("DetectPlayer"), Blackboard->GetValueAsObject(TEXT("LastDamageInstigator")));
				}
			}
		}


	}



	// 선택한 skill의 data를 가지고 와서 distance가 playerdistance 보다 적으면 사용 하도록 함
	//if (Distance < SkillRange)
	//{
	//	Blackboard->SetValueAsBool(TEXT("CanUseSkill"), true);
	//}
	//else
	//{
	//	Blackboard->SetValueAsBool(TEXT("CanUseSkill"), false);
	//}
	//선택된 몽타주 전달
}

void AMonsterAIController::LookTarget()
{
	UObject* DetectedPlayer = Blackboard->GetValueAsObject(TEXT("DetectPlayer"));

	if (DetectedPlayer != nullptr)
	{
		if (AActor* DetectedPlayerActor = Cast<AActor>(DetectedPlayer))
		{
			FVector PlayerLoc = DetectedPlayerActor->GetActorLocation();
			FVector OwnerLoc = GetPawn()->GetActorLocation();
			FVector LookVector = (PlayerLoc - OwnerLoc).GetSafeNormal();
			FVector CurrentForward = GetPawn()->GetActorForwardVector();
			float LookAngle = FVector::DotProduct(CurrentForward, LookVector);
			
			//몽타주 재생중이 아니면
			if (!Blackboard->GetValueAsBool(TEXT("MontagePlaying")))
			{
				// 대상을 바라보고 있지 않으면
				if (LookAngle < 0.89)
				{
					LookOnTarget = false;
					SetFocus(DetectedPlayerActor);	

				}
				else
				{		
					LookOnTarget = true;
				}
			}
			else
			{
				SetFocus(nullptr);
				LookOnTarget = true;
			}
		}
	}

}

void AMonsterAIController::OnAttack()
{
	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		//Casting성공
		UAbilitySystemComponent* MonsterAbilitySystemComponent = OwnerMonster->GetAbilitySystemComponent();

	}



}

void AMonsterAIController::CheckHP()
{
	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		UAbilitySystemComponent* MonsterASC = OwnerMonster->GetAbilitySystemComponent();
		float m_fHealth = Cast<UNAAttributeSet>(MonsterASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();
		Blackboard->SetValueAsFloat(TEXT("HP"), m_fHealth);
	}
}
