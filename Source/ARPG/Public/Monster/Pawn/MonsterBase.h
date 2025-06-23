// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../AI/MonsterAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Aicontroller.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "CoreMinimal.h"

#include "AbilitySystemInterface.h"
#include "Logging/LogMacros.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "MonsterBase.generated.h"


//DECLARE_LOG_CATEGORY_EXTERN(LogTemplateMonster, Log, All);


struct FGameplayEffectModCallbackData;
struct FOnAttributeChangeData;

UENUM(BlueprintType)
enum class MonsterRate : uint8
{
	MR_None			UMETA(DisplayName = "None"),		//	초기화
	MR_Normal		UMETA(DisplayName = "Normal"),		//	일반몹
	MR_Named		UMETA(DisplayName = "Named"),		//	아이템 드랍할 정도는 되는 몹
	MR_Seed			UMETA(DisplayName = "Seed"),		//	SubBoss 정도? 간단한 스킬 하나 정도?
	MR_Boss			UMETA(DisplayName = "Boss"),		//	보스


};

//Monster 도 경국 ability system을 사용을 해서 공격이나 다른걸 사용하니 얘도 component 붙여야 할거 같음
class UAbilitySystemComponent;
class UAbilityTask_PlayMontageAndWait;
class UNAMontageCombatComponent;
class UNAAttributeSet;

UCLASS()
class ARPG_API AMonsterBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMonsterBase();

	void InitializeAbilities();

	virtual void PossessedBy(AController* NewController) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Attribute Setting from other data
	virtual void SetAttributeData(const FDataTableRowHandle& InDataTableRowHandle);

	/* Gas 전환중 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//Gas 전환 완료
	//virtual bool OnAttack();

	//Take damage Parts
	//virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void TakeDamageStun(float HP, UAnimMontage* TakeDamageMontage, float StunSpeed);

	UFUNCTION()
	virtual void OnDie();
	void BodySlash();

	void DropItem(const FDataTableRowHandle& InDataTableRowHandle);

	virtual void initializeAttribute(const FOnAttributeChangeData& Data);
	virtual void OnHealthDepleted();

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float GetBaseDamage() { return BaseDamage; }

	//Gas에서 호출하는 함수들은 여기에 사용하는게 좋아보임
public:
	FORCEINLINE UAbilitySystemComponent* GetAbilitySystemComponent() const override{ return AbilitySystemComponent; }

	UAnimMontage* GetAttackMontage() const { return TestAttackMontage; }
	TArray<UAnimMontage*> GetAttackMontageCombo() const { return AttackComboMontage; }
	UAnimMontage* GetSpawnMontage() const { return SpawnMontage; }

	FDataTableRowHandle GetSkillData() const { return OwnSkills; }
	int GetComboState() const { return SelectedCombo; }
	void SaveCombo() { ++SelectedCombo; }
	void ResetCombo() { SelectedCombo = 0; }

	void SetSelectSkillMontage(UAnimMontage* SelectSkillMontage) { CurrentSkillMontage = SelectSkillMontage; }
	UAnimMontage* GetSelectSkillMontage() const { return CurrentSkillMontage; }
	UAnimMontage* GetDeathMontage() const { return DeathMontage; }
	UAnimMontage* GetSuplexedMontage() const { return SuplexedMontage; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UFloatingPawnMovement* MovementComponent;

protected:
	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere)
	UAISenseConfig_Sight* AISenseConfig_Sight;

	float CheckTimer = 0;
	float CheckHP = 0;
	int	SelectedCombo = 0;
	float BaseDamage = 0;
	//이거 데이터화 시키고 get을 데이터테이블로 보내는게 낫지않나? 싶은데...
public:
	//ComboAttack
	UPROPERTY(EditAnywhere)
	TArray<UAnimMontage*> AttackComboMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* TestAttackMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* SpawnMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* CurrentSkillMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* DeathMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* SuplexedMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* DamageMontage;

	UPROPERTY(EditAnywhere, meta = (RowType = "/Script/ARPG.OwnSkillTable"))
	FDataTableRowHandle OwnSkills;

	UPROPERTY(EditAnywhere, meta = (RowType = "/Script/ARPG.MonsterOwnTable"))
	FDataTableRowHandle OwnStatData;
};
