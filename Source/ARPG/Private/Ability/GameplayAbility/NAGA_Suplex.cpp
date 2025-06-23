// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Suplex.h"
#include "NACharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/ActorComponent/NACombatComponent.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraActor.h"
#include "Kismet/KismetMathLibrary.h"

UNAGA_Suplex::UNAGA_Suplex()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Status.Suplex"));

}

void UNAGA_Suplex::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AvatarActor.Get()->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>())
		{
			//ActorInfo->AbilitySystemComponent->PlayMontage
			//(
			//	this,
			//	ActivationInfo,
			//	CombatComponent->GetGrabMontage(),
			//	CombatComponent->GetMontagePlayRate()
			//);

			// Camera Action도 넣고 싶어지네 뭔가..
			ASC;
			ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor);
			Character->GetCharacterMovement()->DisableMovement();
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, CombatComponent->GetGrabMontage(), 1.0f);
			MontageTask->OnCompleted.AddDynamic(this, &UNAGA_Suplex::OnMontageFinished);
			MontageTask->ReadyForActivation();


			APlayerController* PlayerController = Cast<APlayerController>(ActorInfo->PlayerController);



			FVector StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
			FRotator StartRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

			
			ACameraActor* ActionCamA = GetWorld()->SpawnActor<ACameraActor>(StartLocation, StartRotation);
			if (!ActionCamA) return;
			ACameraActor* ActionCamB = GetWorld()->SpawnActor<ACameraActor>(StartLocation, StartRotation);
			if (!ActionCamB) return;
			ACameraActor* ActionCamC = GetWorld()->SpawnActor<ACameraActor>(StartLocation, StartRotation);
			if (!ActionCamC) return;


			FVector RightVec = Character->GetActorRightVector();
			FVector ForwardVec = Character->GetActorForwardVector();
			FVector UpVec = Character->GetActorUpVector();

			FVector LocationA = Character->GetActorLocation() + RightVec * 120;
			FRotator RotationA = UKismetMathLibrary::FindLookAtRotation(LocationA, Character->GetActorLocation());
			FVector LocationB = Character->GetActorLocation() + ForwardVec * 160+UpVec * 80;
			FVector LocationC = Character->GetActorLocation() + ForwardVec * -200 + RightVec * -250 + UpVec * 100;
			FRotator RotationB = UKismetMathLibrary::FindLookAtRotation(LocationB, Character->GetActorLocation());
			// B 카메라 전환
			ActionCamB->SetActorLocationAndRotation(LocationB, RotationB);
			FRotator RotationC = UKismetMathLibrary::FindLookAtRotation(LocationC, Character->GetActorLocation());
			// B 카메라 전환
			ActionCamC->SetActorLocationAndRotation(LocationC, RotationC);
			// Test용 바로 setting
			ActionCamA->SetActorLocationAndRotation(LocationA, RotationA);
			PlayerController->SetViewTargetWithBlend(ActionCamA, 1.f);
			FTimerHandle CameraBHandle;
			GetWorld()->GetTimerManager().SetTimer(CameraBHandle, FTimerDelegate::CreateLambda([=]()
				{
					PlayerController->SetViewTargetWithBlend(ActionCamB, 1.f);				
				}), 1.0f, false); // 2초 뒤 실행

			FTimerHandle CameraBHandle4;
			GetWorld()->GetTimerManager().SetTimer(CameraBHandle4, FTimerDelegate::CreateLambda([=]()
				{
					PlayerController->SetViewTargetWithBlend(ActionCamC, 1.f);
				}), 3.0f, false); // 2초 뒤 실행


			FTimerHandle CameraBHandle2;
			GetWorld()->GetTimerManager().SetTimer(CameraBHandle2, FTimerDelegate::CreateLambda([=]()
				{
					PlayerController->SetViewTargetWithBlend(Character, 1.4f, EViewTargetBlendFunction::VTBlend_EaseInOut,2.0f, true);
				}), 4.1f, false); // 2초 뒤 실행
			FTimerHandle CameraBHandle3;
			GetWorld()->GetTimerManager().SetTimer(CameraBHandle3, FTimerDelegate::CreateLambda([=]()
				{					
					ActionCamA->Destroy();
					ActionCamB->Destroy();	
					ActionCamC->Destroy();	
				}), 6.0f, false); // 2초 뒤 실행

		}

	}
}

void UNAGA_Suplex::OnMontageFinished()
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);


		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());


		PlayerController->SetViewTargetWithBlend(GetAvatarActorFromActorInfo(), 1.f);
		


	}
}

void UNAGA_Suplex::IgnoreDamage(FGameplayTag GameplayTag, int Count)
{
}
