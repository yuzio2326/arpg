// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Assets/Interface/NAManagedAsset.h"
#include "Combat/Interface/NAHandActor.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NACharacter.generated.h"

class UNAReviveWidgetComponent;
class UWidgetComponent;
class UNAVitalCheckComponent;
class UNAMontageCombatComponent;
class UNAAttributeSet;
class UGameplayEffect;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANACharacter : public ACharacter, public IAbilitySystemInterface, public INAManagedAsset, public INAHandActor
{
	GENERATED_BODY()
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = Gameplay, meta=(AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UChildActorComponent* LeftHandChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UChildActorComponent* RightHandChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UNAVitalCheckComponent* VitalCheckComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UNAReviveWidgetComponent* ReviveWidget;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LeftMouseAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RightMouseAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReviveAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asset", meta=(AllowPrivateAccess="true"))
	FName AssetName;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UNAInteractionComponent> InteractionComponent;

	/* Interaction Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractionAction;

	/* Inventory*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InventoryAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UNAInventoryComponent> InventoryComponent;

	/* Grab */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GrabAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> InventoryWidgetBoom;

	// 인벤토리 활성 시, FollowCamera를 부착할 때 사용하는 스프링 암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> InventoryAngleBoom;

	/* Inventory IMC */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InventoryMappingContext;

	// 양손에 무기가 없을때 사용되는 전투 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Combat", meta=(AllowPrivateAccess="true"))
	UNAMontageCombatComponent* DefaultCombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "AnimInstance", meta = (AllowPrivateAccess = "true"))
	bool IsZoom;
public:
	ANACharacter();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void SetAssetNameDerivedImplementation(const FName& InAssetName) override { AssetName = InAssetName; }

	virtual FName GetAssetName() const override { return AssetName; }

	virtual void RetrieveAsset(const AActor* InCDO) override;

	virtual void OnRep_PlayerState() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// 왼쪽 클릭으로 공격을 시작할 경우
	UFUNCTION()
	void StartLeftMouseAttack();

	// 오른 클릭으로 공격을 시작할 경우
	UFUNCTION()
	void StopLeftMouseAttack();

	// Interaction Input
	UFUNCTION()
	void TryInteract();

	// Inventory Input
	UFUNCTION()
	void ToggleInventoryWidget();

	// 오른쪽 클릭으로 줌인 
	UFUNCTION()
	void ZoomIn();

	void Zoomcheck();
	
	void ChangeCameraAngle(USpringArmComponent* NewBoom, float OverTime);
	
protected:
	void TryRevive();

	void StopRevive();
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// To add mapping context
	virtual void BeginPlay() override;

	virtual void PostNetInit() override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION( Server, Reliable )
	void Server_RequestReviveAbility();

protected:
	virtual UChildActorComponent* GetLeftHandChildActorComponent() const override { return LeftHandChildActor; }
	virtual UChildActorComponent* GetRightHandChildActorComponent() const override { return RightHandChildActor; }
	
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE UNAReviveWidgetComponent* GetReviveWidget() const { return ReviveWidget; }
};

