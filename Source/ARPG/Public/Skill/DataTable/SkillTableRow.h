// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillTableRow.generated.h"

/**
 * 
 */
USTRUCT()
struct ARPG_API FSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	//Skill Montage
	UPROPERTY(EditAnywhere)
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere)
	float Damage;
	UPROPERTY(EditAnywhere)
	float Cooltime;
	UPROPERTY(EditAnywhere)
	float Cost;

	// Monster가 사용시 사용 가능한 사거리
	UPROPERTY(EditAnywhere)
	float Range;

	//SpawnProjectile (Projectile Data를 만들고 Spawn시키도록 해야함 -> 그런데 쓸지 안쓸지를 모르겠어어 일단 주석)

	
	
	
};


// 하나의 Pawn(Character)가 보유중인 Skill
USTRUCT()
struct FOwnSkillTable : public FTableRowBase
{
	GENERATED_BODY()
public:
	//Array로 가지고 갈 스킬들을 가져가기
	//UPROPERTY(EditAnywhere, meta = (RowType = "/Script/ARPG.SkillTable"))
	//TArray<FDataTableRowHandle> OwnSkillArray;

	// 여기서 Array로 만들어가지고 monster한테 전달하는 방식으로 ㄱㄱ
	UPROPERTY(EditAnywhere)
	TArray<FSkillTableRow> OwnSkillArray;


};

// Item 및 Monster 가 사용할 Projectile
USTRUCT()
struct FProjectileTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 만들어야 하네?
public:
	//UPROPERTY(EditAnywhere, Category = "Projectile")
	//TSubclassOf<AProjectile> ProjectileClass;

public:
	UPROPERTY(EditAnywhere, Category = "Projectile")
	UStaticMesh* StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float InitialSpeed = 0;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float MaxSpeed = 0;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float InitialLifeSpan = 5;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float Damage = 0;

};