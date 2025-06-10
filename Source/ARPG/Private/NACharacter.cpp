// Copyright Epic Games, Inc. All Rights Reserved.

#include "NACharacter.h"
#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "NAPlayerState.h"
#include "ARPG/ARPG.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "HP/ActorComponent/NAVitalCheckComponent.h"
#include "HP/GameplayAbility/NAGA_Revive.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "HP/WidgetComponent/NAReviveWidgetComponent.h"
#include "Net/UnrealNetwork.h"

#include "Interaction/NAInteractionComponent.h"
#include "Inventory/NAInventoryComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DefaultAnimInstance.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AARPGCharacter

// 공격시 반복되는 함수 패턴
bool TryAttack( const AActor* Hand )
{
	if ( Hand )
	{
		if ( UNAMontageCombatComponent* CombatComponent = Hand->GetComponentByClass<UNAMontageCombatComponent>() )
		{
			if ( CombatComponent->IsAbleToAttack() )
			{
				CombatComponent->StartAttack();
				return true;
			}
		}
	}

	return false;
}

bool TryStopAttack( const AActor* Hand )
{
	if ( Hand )
	{
		if ( UNAMontageCombatComponent* CombatComponent = Hand->GetComponentByClass<UNAMontageCombatComponent>() )
		{
			if ( CombatComponent->IsAttacking() )
			{
				CombatComponent->StopAttack();
				return true;
			}
		}
	}

	return false;
}

ANACharacter::ANACharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionObjectType( ECC_Pawn );
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	DefaultCombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>( TEXT( "DefaultCombatComponent" ) );
	InteractionComponent = CreateDefaultSubobject<UNAInteractionComponent>(TEXT("InteractionComponent"));
	InventoryWidgetBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("InventorySpringArm"));
	InventoryWidgetBoom->SetupAttachment(RootComponent);
	InventoryWidgetBoom-> bUsePawnControlRotation = false;
	InventoryWidgetBoom-> bInheritPitch = false;
	InventoryWidgetBoom-> bInheritYaw = false;
	InventoryWidgetBoom-> bInheritRoll = false;
	InventoryWidgetBoom->SetRelativeRotation(FRotator(0.0f, 190.0f, 0.0f));
	InventoryWidgetBoom->TargetArmLength = 10.f;
	InventoryWidgetBoom->bDoCollisionTest = false;
	InventoryComponent = CreateDefaultSubobject<UNAInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetupAttachment(InventoryWidgetBoom, USpringArmComponent::SocketName);
	InventoryAngleBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("InventoryAngleBoom"));
	InventoryAngleBoom->SetupAttachment(RootComponent);
	InventoryAngleBoom-> bUsePawnControlRotation = true;
	InventoryAngleBoom-> bInheritPitch = true;
	InventoryAngleBoom-> bInheritYaw = true;
	InventoryAngleBoom-> bInheritRoll = false;
	
	LeftHandChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("LeftHandChildActor"));
	RightHandChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("RightHandChildActor"));

	VitalCheckComponent = CreateDefaultSubobject<UNAVitalCheckComponent>(TEXT("VitalCheckComponent"));
	ReviveWidget = CreateDefaultSubobject<UNAReviveWidgetComponent>( TEXT("ReviveWidgetComponent") );

	if ( GetMesh()->GetSkeletalMeshAsset() )
	{
		LeftHandChildActor->SetupAttachment(GetMesh(), LeftHandSocketName);
		RightHandChildActor->SetupAttachment(GetMesh(), RightHandSocketName);
		ReviveWidget->SetupAttachment( GetMesh(), TEXT("ReviveWidgetSocket") );
	}

	GetMesh()->SetIsReplicated( true );
	bReplicates = true;
	ACharacter::SetReplicateMovement( true );
}

void ANACharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

}

void ANACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AbilitySystemComponent->SetNetAddressable();
	DefaultCombatComponent->SetNetAddressable();
}

void ANACharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// 클라이언트의 BeginPlay에 맞춰 직접 RPC로 요청
	// 서버에서 직접 수행할 경우 클라이언트에서의 순서:
	// - Character -> BeginPlay -> GiveAbility -> PlayerState -> AbilityComponent 초기화
	// 초기화가 되지 않은 시점에서의 GiveAbility의 Replication을 받은 Client은 제대로된 값을 받지 못함
	if ( GetController() == GetWorld()->GetFirstPlayerController() )
	{
		Server_RequestReviveAbility();
	}
	
	// == 테스트 코드 ==
	{
		if (HasAuthority())
		{
			// 기본 공격이 정의되어있지 않음!
			check( DefaultCombatComponent->GetMontage() && DefaultCombatComponent->GetAttackAbility() );
			// Grap Ability 를 만들어야 하네?~
			//check( DefaultCombatComponent->GetGrabMontage() && DefaultCombatComponent->GetAttackAbility() );
			
			// 데미지
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddInstigator(GetController(), this);

			// Gameplay Effect CDO, 레벨?, ASC에서 부여받은 Effect Context로 적용할 효과에 대한 설명을 생성
			const FGameplayEffectSpecHandle DamageEffectSpec = AbilitySystemComponent->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1, EffectContext);

			// 설명에 따라 효과 부여 (본인에게)
			const auto& Handle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec.Data.Get());
			// 다른 대상에게...
			//AbilitySystemComponent->ApplyGameplayEffectSpecToTarget()
			check(Handle.WasSuccessfullyApplied());
		}
	}
	// ===============
}

void ANACharacter::PostNetInit()
{
	Super::PostNetInit();
}

void ANACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if ( HasAuthority() )
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
		}

		SetOwner(NewController);
	}

	if (InventoryComponent && NewController->IsLocalPlayerController())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				InventoryComponent->SetOwnerPlayer(LocalPlayer);
			}
		}
	}
}

void ANACharacter::Server_RequestReviveAbility_Implementation()
{
	// 부활 기능 추가
	const FGameplayAbilitySpec SpecHandle( UNAGA_Revive::StaticClass(), 1.f, static_cast<int32>( EAbilityInputID::Revive ) );
	AbilitySystemComponent->GiveAbility( SpecHandle );
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(InventoryMappingContext, 1);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANACharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANACharacter::Look);

		EnhancedInputComponent->BindAction(LeftMouseAttackAction, ETriggerEvent::Started, this, &ANACharacter::StartLeftMouseAttack);
		EnhancedInputComponent->BindAction(LeftMouseAttackAction, ETriggerEvent::Completed, this, &ANACharacter::StopLeftMouseAttack);
		
		EnhancedInputComponent->BindAction( ReviveAction, ETriggerEvent::Started, this, &ANACharacter::TryRevive );
		EnhancedInputComponent->BindAction( ReviveAction, ETriggerEvent::Completed, this, &ANACharacter::StopRevive );
		// Interact
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ANACharacter::TryInteract);
		// Inventory
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ANACharacter::ToggleInventoryWidget);

		// Grab			TryInteract에서 cast 된 대상이 monster일 경우 활성화 시키면 될거 같음
		//EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, &ANACharacter::TryInteract);
		EnhancedInputComponent->BindAction(RightMouseAttackAction, ETriggerEvent::Started, this, &ANACharacter::ZoomIn);


	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANACharacter::RetrieveAsset(const AActor* InCDO)
{
	if (const ANACharacter* DefaultAsset = Cast<ANACharacter>(InCDO))
	{
		// 블루프린트의 컴포넌트 속성 복사
		for ( TFieldIterator<FObjectProperty> It(GetClass()); It; ++It  )
		{
			if ( It->PropertyClass->IsChildOf( UActorComponent::StaticClass() ) &&
				 !It->PropertyClass->IsChildOf( UInputComponent::StaticClass() ) )
			{
				UActorComponent* ThisComponent = Cast<UActorComponent>( It->GetObjectPropertyValue_InContainer( this ) );
				UActorComponent* OriginComponent = Cast<UActorComponent>( It->GetObjectPropertyValue_InContainer( DefaultAsset ) );

				if ( !ThisComponent || !OriginComponent )
				{
					continue;
				}
				
				// 프로퍼티 복사 후 컴포넌트 재등록하면서 갱신
				ThisComponent->UnregisterComponent();
				if ( IsValid( GEngine ) )
				{
					GEngine->CopyPropertiesForUnrelatedObjects( OriginComponent, ThisComponent );	
				}

				// Scene Component이면...
				if ( USceneComponent* OriginSceneComponent = Cast<USceneComponent>( OriginComponent ) )
				{
					// 부모로 부착된 컴포넌트가 있는지 확인하고...
					if ( USceneComponent* OriginParentComponent = OriginSceneComponent->GetAttachParent() )
					{
						bool bAttached = false;
						// 똑같은 컴포넌트를 프로퍼티로 찾아서...
						for ( TFieldIterator<FObjectProperty> ParentIt( GetClass() ); ParentIt; ++ParentIt )
						{
							if ( ParentIt->PropertyClass->IsChildOf( USceneComponent::StaticClass() ) )
							{
								USceneComponent* ComponentPtrFromOrigin = Cast<USceneComponent>( ParentIt->GetObjectPropertyValue_InContainer( DefaultAsset ) );

								// 이 객체의 프로퍼티에 있는 컴포넌트에 똑같이 적용
								if ( ComponentPtrFromOrigin == OriginParentComponent )
								{
									USceneComponent* ThisParentComponent = Cast<USceneComponent>( ParentIt->GetObjectPropertyValue_InContainer( this ) );
									USceneComponent* ThisSceneComponent = Cast<USceneComponent>( ThisComponent );
									ThisSceneComponent->AttachToComponent( ThisParentComponent, FAttachmentTransformRules::KeepRelativeTransform, OriginSceneComponent->GetAttachSocketName() );
									bAttached = true;
									break;
								}
							}
						}

						// 모종의 이유로 부착에 실패
						check( bAttached );
					}
				}
				
				ThisComponent->RegisterComponent();
			}
		}

		FObjectPropertyUtility::CopyClassPropertyIfTypeEquals<ANACharacter, UInputMappingContext, UInputAction>( this, DefaultAsset );

		const FTransform Transform = DefaultAsset->GetMesh()->GetRelativeTransform();
		
		GetMesh()->SetRelativeTransform(Transform);
		// 리플리케이션을 위한 Mesh Offset
		CacheInitialMeshOffset( Transform.GetLocation(), Transform.Rotator() );
	}
}

void ANACharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	GetAbilitySystemComponent()->InitAbilityActorInfo
	(
		GetPlayerState<ANAPlayerState>(),
		this
	);
}

void ANACharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANACharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANACharacter::StartLeftMouseAttack()
{
	if ( !TryAttack( RightHandChildActor->GetChildActor() ) )
	{
		if ( DefaultCombatComponent->IsAbleToAttack() && !DefaultCombatComponent->IsAttacking() )
		{
			DefaultCombatComponent->StartAttack();	
		}
	}
}

void ANACharacter::StopLeftMouseAttack()
{
	if ( !TryStopAttack( RightHandChildActor->GetChildActor() ) )
	{
		if ( DefaultCombatComponent->IsAttacking() )
		{
			DefaultCombatComponent->StopAttack();	
		}
	}
}

void ANACharacter::TryInteract()
{
	if (ensure(InteractionComponent != nullptr))
	{
		InteractionComponent->BeginInteraction();
	}
}

void ANACharacter::ToggleInventoryWidget()
{
	if (ensure(InventoryComponent != nullptr))
	{
		if (InventoryComponent->IsInventoryWidgetVisible())
		{
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->SetIgnoreLookInput(false);
				ChangeCameraAngle(CameraBoom, 2.f);
				InventoryComponent->CollapseInventoryWidget();
			}
		}
		else
		{
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->SetIgnoreLookInput(true);
				ChangeCameraAngle(InventoryAngleBoom, 2.f);
				InventoryComponent->ReleaseInventoryWidget();
			}
		}
	}
}

void ANACharacter::ZoomIn()
{		
	UDefaultAnimInstance* PlayerAniminstance = Cast<UDefaultAnimInstance>(GetMesh()->GetAnimInstance());

	bool boolCheck = false;

	//리슨서버 호스트에서 작동
	if (HasAuthority())
	{
		bool ReceiveServer = true;

		if (!IsZoom)
		{
			//Zoom 상태 돌입
			//GetMesh()->SetOwnerNoSee(true);
			FollowCamera->AddRelativeLocation(FVector(200, 0, 30));
			IsZoom = true;
			bUseControllerRotationYaw = true;
		}
		else if (IsZoom)
		{
			//GetMesh()->SetOwnerNoSee(false);
			FollowCamera->AddRelativeLocation(FVector(-200, 0, -30));
			IsZoom = false;
			bUseControllerRotationYaw = false;
		}
	}
	// 클라에서 작동
	else
	{
		//ServerZoomIn(); // 클라이언트에서 서버로 요청
		bool SendToServer = true;

		if (!IsZoom)
		{
			//Zoom 상태 돌입
			//GetMesh()->SetOwnerNoSee(true);
			FollowCamera->AddRelativeLocation(FVector(200, 0, 30));
			IsZoom = true;
			bUseControllerRotationYaw = true;
		}
		else if (IsZoom)
		{
			//GetMesh()->SetOwnerNoSee(false);
			FollowCamera->AddRelativeLocation(FVector(-200, 0, -30));
			IsZoom = false;
			bUseControllerRotationYaw = false;
		}
	}

	//아이템 열었을때 줌 못하도록 하기
	//Changed Pressed
	//UpperBody 사용하도록 바꿔야함 + 상체 고정 필요
	
	
}

void ANACharacter::ChangeCameraAngle(USpringArmComponent* NewBoom, float OverTime)
{
	if (NewBoom)
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			FollowCamera->AttachToComponent(NewBoom, FAttachmentTransformRules::KeepRelativeTransform, USpringArmComponent::SocketName);
			FVector SpringArmSocketLocation = FVector::ZeroVector;
												//NewBoom->GetSocketLocation(USpringArmComponent::SocketName);
			FRotator SpringArmSocketRotation = FRotator::ZeroRotator;
												//NewBoom->GetSocketRotation(USpringArmComponent::SocketName);
		
			FLatentActionInfo LatentInfo;
			UKismetSystemLibrary::MoveComponentTo(FollowCamera, SpringArmSocketLocation, SpringArmSocketRotation, true, true, OverTime, true, EMoveComponentAction::Move, LatentInfo);
			
		}
	}
}

void ANACharacter::TryRevive()
{
	// todo: GAS의 Input 감지를 이용할 수 있음
	if ( AbilitySystemComponent )
	{
		AbilitySystemComponent->AbilityLocalInputPressed( static_cast<int32>( EAbilityInputID::Revive ) );
	}
}

void ANACharacter::StopRevive()
{
	// todo: GAS의 Input 감지를 이용할 수 있음
	if ( AbilitySystemComponent )
	{
		AbilitySystemComponent->AbilityLocalInputReleased( static_cast<int32>( EAbilityInputID::Revive ) );
	}
}

void ANACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( ANACharacter, AbilitySystemComponent );
	DOREPLIFETIME( ANACharacter, DefaultCombatComponent );
	DOREPLIFETIME( ANACharacter, IsZoom);
}
