// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotify/NAAnimNotify_Projectile.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NACharacter.h"
#include "Monster/Pawn/MonsterBase.h"
//Projectile
#include "Skill/DataTable/SkillTableRow.h"



UNAAnimNotify_Projectile::UNAAnimNotify_Projectile()
{
}

FRotator UNAAnimNotify_Projectile::GetPlayerProjectileRotation(const FVector& InProjectSpawnLocation, UCameraComponent* InCameraComponent)
{
	const FVector CameraForwardVector = InCameraComponent->GetForwardVector();
	const FVector DestinationLocation = InCameraComponent->GetComponentLocation() + CameraForwardVector * 5000.0;
		
	TArray<AActor*> IgnoreActors; IgnoreActors.Add(InCameraComponent->GetOwner());
	FHitResult HitResult;

	const ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel4);
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(InCameraComponent->GetWorld(),
		InCameraComponent->GetComponentLocation(), DestinationLocation, TraceTypeQuery,
		false, IgnoreActors, EDrawDebugTrace::None, HitResult, true);

	FRotator Rotator;
	if (bHit)
	{
		Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, HitResult.ImpactPoint);
	}
	else
	{
		Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, DestinationLocation);
	}
	return Rotator;
}

FRotator UNAAnimNotify_Projectile::GetPawnProjectileRotation(const FVector& InProjectSpawnLocation, USkeletalMeshComponent* InSkeletalMeshComponent)
{
	const FVector CameraForwardVector = InSkeletalMeshComponent->GetForwardVector();
	const FVector DestinationLocation = InSkeletalMeshComponent->GetComponentLocation() + CameraForwardVector * 5000.0;

	TArray<AActor*> IgnoreActors; IgnoreActors.Add(InSkeletalMeshComponent->GetOwner());
	FHitResult HitResult;

	const ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel4);
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(InSkeletalMeshComponent->GetWorld(),
		InSkeletalMeshComponent->GetComponentLocation(), DestinationLocation, TraceTypeQuery,
		false, IgnoreActors, EDrawDebugTrace::None, HitResult, true);

	FRotator Rotator;
	if (bHit)
	{
		Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, HitResult.ImpactPoint);
	}
	else
	{
		Rotator = UKismetMathLibrary::FindLookAtRotation(InProjectSpawnLocation, DestinationLocation);
	}
	return Rotator;
}

void UNAAnimNotify_Projectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
#if WITH_EDITOR
	if (GIsEditor && MeshComp->GetWorld() != GWorld) { return; } // 에디터 프리뷰
#endif

	AActor* OwningPawn = MeshComp->GetOwner();

	// Server 에서 Spawn 할수 있도록
	if (OwningPawn->HasAuthority())
	{
		//Socket 기준 location 을 받아서 spawn하도록 하기
		const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
		const FRotator SocketRotator = MeshComp->GetSocketRotation(SocketName);

		FRotator ProjectileRotator = FRotator::ZeroRotator;

		//FProjectileTableRow* ProjectileTableRow;

		//Camera 보유시 Player 아닐시 AI 로 spawn Location Rotation 조절
		if (UCameraComponent* CameraComponent = OwningPawn->GetComponentByClass<UCameraComponent>())
		{
			ProjectileRotator = GetPlayerProjectileRotation(SocketLocation, CameraComponent);

			//projectile data받아서 사용 해야 하는데... 만들어야 함..
			//projectile
			

		}
		//Monster 
		else 
		{

		}


	}

#pragma region Legacy


	//Owner 가지고 와서 
	//APawn* OwningPawn = Cast<APawn>(MeshComp->GetOwner());
	//check(OwningPawn);

	USkeletalMeshComponent* OwnerSkeletalMeshComponent = OwningPawn->GetComponentByClass<USkeletalMeshComponent>();
	check(OwnerSkeletalMeshComponent);

	const FVector MuzzleLocation = OwnerSkeletalMeshComponent->GetSocketLocation(SocketName);
	const FRotator MuzzleRotation = OwnerSkeletalMeshComponent->GetSocketRotation(SocketName);
#if WITH_EDITOR
	USkeletalMeshSocket const* SkeletalMeshSocket = OwnerSkeletalMeshComponent->GetSocketByName(SocketName);
	check(SkeletalMeshSocket);
#endif

	FRotator ProjectileRotator = FRotator::ZeroRotator;
	bool bIsPlayer = false;
	//Camera 보유시 Player 아닐시 AI 로 spawn Location Rotation 조절
	if (UCameraComponent* CameraComponent = OwningPawn->GetComponentByClass<UCameraComponent>())
	{
		bIsPlayer = true;
		ProjectileRotator = GetPlayerProjectileRotation(MuzzleLocation, CameraComponent);
	}
	else
	{
		//Socket 의 tranform 배치함
		bIsPlayer = false;
		FVector ForwardDirection = MuzzleRotation.Vector();
		FRotator ForwardRotation = ForwardDirection.Rotation();
		ProjectileRotator = ForwardRotation;
		//AMonsterAIController* OwnMonsterAI = Cast<AMonsterAIController>(OwningPawn->GetController());

	}

	//Player일 경우
	if (bIsPlayer)
	{
		// ITem 에서 Damage 뽑아 와서 집어 넣어야 할거 같음
		
		//FProjectileTableRow* ProjectileTableRow;
		ANACharacter* PlayerCharacter = Cast<ANACharacter>(OwningPawn);
		
		//기본 스킬에 평타 넣으면 되려나?	

		/*const FProjectileTableRow* ProjectileTableRow = GunTableRow->ProjectileRowHandle.GetRow<FProjectileTableRow>(TEXT("Projectile"));
		check(ProjectileTableRow);*/
		
	}
	else
	{
		// monster의 SkilldataTable 사용 또는 anim notify 에 있는 데미지 사용둘중 하나로 정해야 할거 같음
		
		// Projectile 종류 모아 놓는 Datatable필요
		//FProjectileTableRow* ProjectileTableRow;
				
		{
			AMonsterBase* MonsterPawn = Cast<AMonsterBase>(OwningPawn);
			check(MonsterPawn);
			//Spawn할 Projectile
			//CurrentSkilldata = MonsterSkillComponent->GetCurrentSkillData();
			//ProjectileTableRow = CurrentSkilldata.ProjectileRowHandle.GetRow<FProjectileTableRow>(TEXT("SkillProjectile"));
			//check(ProjectileTableRow);

			//UStatusComponent* MonsterStatus = MonsterPawn->GetMonsterStatus();
			//if (CurrentSkilldata.IsUseStatusSTR)
			//{
			//	float MonsterStat = MonsterStatus->GetSTR();
			//	OwnerDamage = CurrentSkilldata.BonusDamage * MonsterStat;
			//	BaseDamage = CurrentSkilldata.Damage;
			//}
			//else
			//{
			//	float MonsterStat = MonsterStatus->GetINT();
			//	OwnerDamage = CurrentSkilldata.BonusDamage * MonsterStat;
			//	BaseDamage = CurrentSkilldata.Damage;
			//}

		}


		UWorld* World = OwningPawn->GetWorld();
		/*AProjectile* Projectile = World->SpawnActorDeferred<AProjectile>(ProjectileTableRow->ProjectileClass,
			FTransform::Identity, OwningPawn, OwningPawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectile->SetData(CurrentSkilldata.ProjectileRowHandle);
		Projectile->SetStatDamage(BaseDamage, OwnerDamage);*/

		FTransform NewTransform;
		NewTransform.SetLocation(MuzzleLocation);
		NewTransform.SetRotation(ProjectileRotator.Quaternion());
		//Projectile->FinishSpawning(NewTransform);

	}
#pragma endregion


}
