// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NAInventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Inventory/GameInstance/NAInventoryGameInstanceSubsystem.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"
#include "Inventory/Widget/NAInventoryWidget.h"
#include "Item/ItemActor/NAWeapon.h"
#include "Components/Button.h"

// 슬롯 ID에서 문자열 추출 헬퍼 함수
static int32 ExtractSlotNumber(const FName& SlotID)
{
	const FString SlotStr = SlotID.ToString();
	int32 UnderscoreIndex = INDEX_NONE;
	if (SlotStr.FindLastChar(TEXT('_'), UnderscoreIndex))
	{
		const FString NumberStr = SlotStr.Mid(UnderscoreIndex + 1);
		return FCString::Atoi(*NumberStr);
	}
	return INT_MAX;
}

// Sets default values for this component's properties
UNAInventoryComponent::UNAInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UNAInventoryWidget> InventoryWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/Inventory/BP_NAInventoryWidget.BP_NAInventoryWidget_C'"));
	check(InventoryWidgetClass.Class);
	SetWidgetClass(InventoryWidgetClass.Class);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetEnableGravity(false);
	CanCharacterStepUpOn = ECB_No;
	SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawSize(FVector2D(1280, 720));
	SetGeometryMode(EWidgetGeometryMode::Cylinder);
	SetCylinderArcAngle(180.f);
	bIsTwoSided = true;
	SetBlendMode(EWidgetBlendMode::Masked);
	OpacityFromTexture = 0.8f;
	SetVisibility(false);

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// 슬롯 데이터 매핑 키 값만(ID) 초기화
		InitSlotIDs(InventoryContents);
		// 슬롯 UI 매핑 키 값만(ID) 초기화
		InitSlotIDs(SlotButtons);
	}
}


// Called when the game starts
void UNAInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);
	
}

bool UNAInventoryComponent::HandleRemoveItem(const FName& SlotID)
{
	if (IsEmptySlot(SlotID))
	{
		return false;
	}

	UNAItemData* ItemToRemove = InventoryContents[SlotID].Get();
	if (!ItemToRemove) { return false; }
	if (ItemToRemove->GetOwningInventory() != this) { return ensure(false); }
	
	if (UNAInventoryGameInstanceSubsystem* InvenSubsys = UNAInventoryGameInstanceSubsystem::Get(GetWorld()))
	{
		UNAItemData* RemovedItem = InvenSubsys->RemoveItemFromInventory(ItemToRemove);
		if (!RemovedItem || ItemToRemove != RemovedItem) { return ensure(false); }
		RemovedItem->SetOwningInventory(nullptr);
		return true;
	}
	
	return ensure(false);
}

void UNAInventoryComponent::SortInventoryItems()
{
    // 1) 채워진 Inven_ 슬롯만 모으기
    TArray<TPair<FName, UNAItemData*>> FilledSlots;
    FilledSlots.Reserve(MaxInventorySlots);

    for (auto& Pair : InventoryContents)
    {
        const FName& SlotID = Pair.Key;
        FString SlotStr = SlotID.ToString();
        if (SlotStr.StartsWith(TEXT("Inven_")) && Pair.Value.IsValid())
        {
            FilledSlots.Emplace(SlotID, Pair.Value.Get());
        }
    }

    // 2) 정렬: ItemType ↑, ClassName ↑, Quantity ↓
    FilledSlots.Sort([](const TPair<FName, UNAItemData*>& A, const TPair<FName, UNAItemData*>& B) {
        const UNAItemData* ItemA = A.Value;
        const UNAItemData* ItemB = B.Value;
        
        // 2-1) 타입 비교
        EItemType TypeA = ItemA->GetItemType();
        EItemType TypeB = ItemB->GetItemType();
        if (TypeA != TypeB)
        {
            return static_cast<uint8>(TypeA) < static_cast<uint8>(TypeB);
        }
        
        // 2-2) 클래스 이름 비교
        FString ClassNameA = ItemA->GetItemActorClass()->GetName();
        FString ClassNameB = ItemB->GetItemActorClass()->GetName();
        if (ClassNameA != ClassNameB)
        {
            return ClassNameA < ClassNameB;
        }
        
        // 2-3) 같은 클래스면 수량 내림차순
        return ItemA->GetQuantity() > ItemB->GetQuantity();
    });

    // 3) 모든 Inven_ 슬롯을 비우기
    for (int32 i = 1; i <= MaxInventorySlots; ++i)
    {
        FString SlotNameStr = FString::Printf(TEXT("Inven_%02d"), i);
        FName SlotID = FName(*SlotNameStr);
        InventoryContents[SlotID] = nullptr;
    }

    // 4) 정렬된 순서대로 순차적 재배치
    int32 TargetIndex = 1;
    for (auto& Pair : FilledSlots)
    {
        UNAItemData* ItemData = Pair.Value;
        FString NewSlotName = FString::Printf(TEXT("Inven_%02d"), TargetIndex);
        FName NewSlotID = FName(*NewSlotName);

        InventoryContents[NewSlotID] = ItemData;
        
        ++TargetIndex;
    }
	
	// 5) UI 쪽에 갱신 알리기
	// if (IsValid(GetInventoryWidget()))
	// {
	// 	for (const auto& Pair : InventoryContents)
	// 	{
	// 		if (SlotButtons.Find(Pair.Key) != nullptr)
	// 		{
	// 			FName SlotID = Pair.Key;
	// 			UNAItemData* ItemData = Pair.Value.Get();
	// 			UButton* SlotButton = SlotButtons[SlotID].Get();
	// 			GetInventoryWidget()->RefreshSlotButtons(ItemData, SlotButton);
	// 		}
	// 	}
	// }
	
	if (IsValid(GetInventoryWidget()))
	{
		GetInventoryWidget()->RefreshInvenSlotButtons(InventoryContents, SlotButtons);
		UpdateWidget();
	}
}

// void UNAInventoryComponent::SplitExistingStack(UNAItemData* ItemToSplit, const int32 AmountToSplit)
// {
// 	if (InventoryContents.Num() + 1 <= InventorySlotsCapacity)
// 	{
// 		HandleRemoveAmountOfItems(ItemToSplit, AmountToSplit);
// 		HandleAddNewItem(ItemToSplit, AmountToSplit);
// 	}
// }

TArray<UNAItemData*> UNAInventoryComponent::GetInventoryContents() const
{
	TArray<UNAItemData*> InventoryContentsArray;
	for (const auto& Pair : InventoryContents)
	{
		if (Pair.Value.IsValid())
		{
			InventoryContentsArray.Add(Pair.Value.Get());
		}
	}
	return InventoryContentsArray;
}

FNAItemAddResult UNAInventoryComponent::AddNonStackableItem(UNAItemData* InputItem, const TArray<FName>& InEmptySlots)
{
	FString FailReason;
	// 1) 사전 검증: nullptr, MaxSlotStackSize==1인지, Quantity==1인지, MaxInventoryHoldCount 검사 등
	if (!IsValidForNonStackable(InputItem, FailReason))
	{
		return FNAItemAddResult::AddedNone(FText::FromString(FailReason));
	}

	// 2) 빈 슬롯이 하나라도 있어야 추가 가능
	if (InEmptySlots.IsEmpty())
	{
		return FNAItemAddResult::AddedNone(
			FText::FromString(TEXT("인벤토리에 빈 슬롯이 없습니다."))
		);
	}

	// 3) 첫 번째 빈 슬롯에만 추가
	const FName TargetSlot = InEmptySlots[0];
	const bool bSucceed = HandleAddNewItem(InputItem, TargetSlot);
	if (bSucceed)
	{
		// 비-스택형이므로 Always Distributed = 1
		const FText ItemName = FText::FromString(InputItem->GetItemName());
		return FNAItemAddResult::AddedAll(
			1,
			FText::Format(
				FText::FromString(TEXT("'{0}' 아이템을 성공적으로 추가했습니다.")),
				ItemName
			)
		);
	}

	// 4) 만약 HandleAddNewItem에서 실패한 경우
	return FNAItemAddResult::AddedNone(
		FText::FromString(TEXT("알 수 없는 이유로 추가에 실패했습니다."))
	);
}

FNAItemAddResult UNAInventoryComponent::AddStackableItem(UNAItemData* InputItem, const TArray<FName>& PartialSlots, const TArray<FName>& EmptySlots)
{
	// 1) 기본 입력 검증
    if (!InputItem)
    {
        return FNAItemAddResult::AddedNone(
            FText::FromString(TEXT("아이템이 nullptr입니다."))
        );
    }
    const int32 OriginalQuantity = InputItem->GetQuantity();
    if (OriginalQuantity <= 0)
    {
        return FNAItemAddResult::AddedNone(
            FText::FromString(TEXT("추가할 수량이 0 이하입니다."))
        );
    }
    const int32 MaxHold = InputItem->GetMaxInventoryHoldCount();
    if (MaxHold < 0)
    {
        return FNAItemAddResult::AddedNone(
            FText::FromString(TEXT("유효하지 않은 MaxInventoryHoldCount입니다."))
        );
    }

    // 2) Partial 슬롯과 Empty 슬롯 기반으로 실제로 추가 가능한 총량 계산
    int32 AllowedAmount = ComputeDistributableAmount(InputItem, PartialSlots, EmptySlots);
    if (AllowedAmount <= 0)
    {
        return FNAItemAddResult::AddedNone(
            FText::FromString(TEXT("인벤토리 공간 또는 소지 한도가 부족합니다."))
        );
    }

    // 3) 실제 슬롯에 수량 분배 (Partial → Empty)
    // DistributeToSlots: 원본 데이터를 그대로 슬롯에 등록한 경우 - 원본 Quantity 수정 x
	//					  원본에서 복제한 데이터를 슬롯에 등록한 경우 - 원본 Quantity에서 슬롯의 최대 용량만큼 차감
	//					  복제 데이터를 만들어서 슬롯에 아이템 수량을 분배한 뒤,
	//					  -> 빈 슬롯이 남았다: 원본 데이터를 슬롯에 등록 - 원본 Quantity는 해당 슬롯의 용량을 나타냄
	//					  -> 빈 슬롯이 없다: 아이템이 부분 추가되었으므로, 원본 데이터의 Quantity는 차감된 상태로 월드에 잔여
    int32 Distributed = DistributeToSlots(InputItem, AllowedAmount, PartialSlots, EmptySlots);
    if (Distributed <= 0)
    {
        return FNAItemAddResult::AddedNone(
            FText::FromString(TEXT("아이템 분배 중 오류가 발생했습니다."))
        );
    }

    // 4) 결과 메시지: 전부 추가 vs 부분 추가
    const FText ItemName = FText::FromString(InputItem->GetItemName());
	if (Distributed == OriginalQuantity)
	{
		// 원본이 완전히 인벤토리로 옮겨진 케이스 (Quantity는 0)
		return FNAItemAddResult::AddedAll(
			Distributed,
			FText::Format(
				FText::FromString(TEXT("'{0}' 아이템 {1}개를 전부 인벤토리에 추가했습니다.")),
				ItemName,
				Distributed
			)
		);
	}
	else
	{
		// 일부만 옮겨진 케이스 (Quantity는 “남은” 양)
		const int32 Remaining = OriginalQuantity - Distributed;
		return FNAItemAddResult::AddedPartial(
			Distributed,
			FText::Format(
				FText::FromString(TEXT("'{0}' 아이템 중 {1}개만 인벤토리에 추가했습니다. 남은 {2}개")),
				ItemName,
				Distributed,
				Remaining
			)
		);
	}
}

int32 UNAInventoryComponent::GetNumToFillSlot(const FName& SlotID) const
{
	if (!InventoryContents.Contains(SlotID) || IsEmptySlot(SlotID))
	{
		return -1;
	}

	const int32 CurrentQuantity = InventoryContents[SlotID]->GetQuantity();
	const int32 MaxHoldCount = InventoryContents[SlotID]->GetMaxInventoryHoldCount();
	return FMath::Max(MaxHoldCount - CurrentQuantity, 0);
}

// Called every frame
void UNAInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

int32 UNAInventoryComponent::TryAddItem(UNAItemData* ItemToAdd)
{
	if (!ItemToAdd)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TryAddItem] ItemToAdd가 nullptr입니다."));
		return -1;
	}
	if (!ItemToAdd->IsPickableItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TryAddItem] 인벤토리에 소지 불가능한 아이템입니다."));
		return -1;
	}

	const bool bIsStackable = ItemToAdd->IsStackableItem();
	const UClass* ItemClass = ItemToAdd->GetItemActorClass();

	// 원본 요청 수량을 미리 저장
	const int32 OriginalQuantity = ItemToAdd->GetQuantity();
	
	FNAItemAddResult Result;

	if (bIsStackable)
	{
		// ─────────────────────────────────────────────────
		// 1) Stackable인 경우: Partial 슬롯 + Empty 슬롯 수집
		TArray<FName> PartialSlots;
		TArray<FName> EmptySlots;

		GatherPartialSlots(const_cast<UClass*>(ItemClass), PartialSlots);
		GatherEmptySlots(const_cast<UClass*>(ItemClass), EmptySlots);

		// 2) Internal 함수 호출
		Result = AddStackableItem(ItemToAdd, PartialSlots, EmptySlots);
	}
	else
	{
		// ─────────────────────────────────────────────────
		// 2) Non-stackable인 경우: Empty 슬롯만 수집
		TArray<FName> EmptySlots;
		GatherEmptySlots(const_cast<UClass*>(ItemClass), EmptySlots);

		// 3) Internal 함수 호출
		Result = AddNonStackableItem(ItemToAdd, EmptySlots);
	}

	// 반환값을 OriginalQuantity – ActualAmountAdded 형태로 계산
	if (Result.OperationResult == ENAItemAddStatus::IAR_AddedAll ||
		Result.OperationResult == ENAItemAddStatus::IAR_AddedPartial)
	{
		// 성공(전부 혹은 일부 추가)이므로 정렬 실행
		SortInventoryItems();

		const int32 Added = Result.ActualAmountAdded;
		// “전부 추가”인 경우 Added == OriginalQuantity 이므로 (Remaining = 0) 
		// “부분 추가”인 경우 Remaining > 0
		return OriginalQuantity - Added;
	}
	else
	{
		// 전혀 추가되지 못했을 때
		return -1;
	}
}

UNAItemData* UNAInventoryComponent::TryRemoveItem(const FName& SlotID, int32 RequestedAmount)
{
	if (IsEmptySlot(SlotID)) { return nullptr; }
	if (RequestedAmount <= 0) { return nullptr; }
	UNAItemData* ItemToRemove = InventoryContents[SlotID].Get();
	if (!ItemToRemove) { return nullptr; }
	if (ItemToRemove->GetOwningInventory() != this) { ensure(false); return nullptr; }
	if (ItemToRemove->GetItemMaxSlotStackSize() < RequestedAmount) { return nullptr; }
	
	int32 NewQuantity = ItemToRemove->GetQuantity() - RequestedAmount;
	if (NewQuantity <= 0)
	{
		const bool bSucceed = HandleRemoveItem(SlotID);
		if (bSucceed)
		{
			SortInventoryItems();
		}
		return bSucceed ? ItemToRemove : nullptr;
	}
	
	ItemToRemove->SetQuantity(NewQuantity);
	// @TODO: 인벤토리 슬롯 UI에서 수량 표시 부분 리드로우 요청
	return ItemToRemove;
}

bool UNAInventoryComponent::HasItemOfClass(const UClass* ItemClass) const
{
	if (!ItemClass) { return false; }
	for (const auto& Pair : InventoryContents)
	{
		if (Pair.Value.IsValid() && Pair.Value->GetItemActorClass() == ItemClass)
		{
			return true;
		}
	}
	return false;
}

void UNAInventoryComponent::GetSlotIDsWithItemClass(const UClass* ItemClass, TArray<FName>& OutSlotIDs) const
{
	OutSlotIDs.Reset();
	if (!ItemClass) { return; }
	for (const auto& Pair : InventoryContents)
	{
		if (Pair.Value.IsValid() && Pair.Value->GetItemActorClass() == ItemClass)
		{
			OutSlotIDs.Add(Pair.Key);
		}
	}
}

int32 UNAInventoryComponent::GetSlotRemainingStack(const FName& SlotID) const
{
	// 슬롯에 데이터가 없거나, nullptr이거나, IsEmptySlot일 때
	if (!InventoryContents.Contains(SlotID) || IsEmptySlot(SlotID))
	{
		return -1;
	}

	const int32 CurrentStackSize = InventoryContents[SlotID]->GetQuantity();
	const int32 MaxStackSize = InventoryContents[SlotID]->GetItemMaxSlotStackSize();
	return FMath::Max(MaxStackSize - CurrentStackSize, 0); // 0 미만 반환 방지
}

bool UNAInventoryComponent::IsEmptySlot(const FName& SlotID) const
{
	if (!SlotID.IsNone() && InventoryContents.Contains(SlotID))
	{
		return !InventoryContents[SlotID].IsValid();
	}
	return false;
}

void UNAInventoryComponent::GatherEmptyInventorySlots(TArray<FName>& OutEmptySlots) const
{
	OutEmptySlots.Empty();
	OutEmptySlots.Reserve(MaxInventorySlots);

	for (const auto& Pair : InventoryContents)
	{
		// 값이 비어있으면서(FName→값이 nullptr) “Inven_”으로 시작하는 Key만 수집
		const FName& SlotID = Pair.Key;
		if (!Pair.Value.IsValid())
		{
			const FString SlotStr = SlotID.ToString();
			if (SlotStr.StartsWith(TEXT("Inven_")))
			{
				OutEmptySlots.Add(SlotID);
			}
		}
	}
}
void UNAInventoryComponent::GatherEmptyWeaponSlots(TArray<FName>& OutEmptySlots) const
{
	OutEmptySlots.Empty();
	OutEmptySlots.Reserve(MaxWeaponSlots);

	for (const auto& Pair : InventoryContents)
	{
		// 값이 비어있으면서 “Weapon_”으로 시작하는 Key만 수집
		const FName& SlotID = Pair.Key;
		if (!Pair.Value.IsValid())
		{
			const FString SlotStr = SlotID.ToString();
			if (SlotStr.StartsWith(TEXT("Weapon_")))
			{
				OutEmptySlots.Add(SlotID);
			}
		}
	}
}

bool UNAInventoryComponent::IsFullSlot(const FName& SlotID) const
{
	if (!IsEmptySlot(SlotID))
	{
		return GetNumToFillSlot(SlotID) <= 0;
	}
	return false;
}

bool UNAInventoryComponent::HasNoEmptySlot() const
{
	for (const auto& Pair : InventoryContents)
	{
		if (IsEmptySlot(Pair.Key))
		{
			return false;
		}
	}
	return true;
}

bool UNAInventoryComponent::IsAtFullCapacity() const
{
	for (const auto& Pair : InventoryContents)
	{
		if (IsEmptySlot(Pair.Key) || !IsFullSlot(Pair.Key))
		{
			return false;
		}
	}
	return true;
}

void UNAInventoryComponent::GatherPartialSlots(UClass* ItemClass, TArray<FName>& OutPartialSlots) const
{
	if (!ItemClass) { ensure(false); return; }
	
	OutPartialSlots.Empty();
	for (const auto& Pair : InventoryContents)
	{
		if (Pair.Value.IsValid() && Pair.Value->GetItemActorClass() == ItemClass)
		{
			// 해당 슬롯의 현재 수량이 최대 슬롯 스택에 도달하지 않았으면 후보로 추가
			const int32 CurrQty = Pair.Value->GetQuantity();
			const int32 MaxSlotStack = Pair.Value->GetItemMaxSlotStackSize();
			if (CurrQty < MaxSlotStack)
			{
				OutPartialSlots.Add(Pair.Key);
			}
		}
	}
}

void UNAInventoryComponent::GatherEmptySlots(UClass* ItemClass, TArray<FName>& OutEmptySlots) const
{
	if (!ItemClass) { ensure(false); return; }
	
	OutEmptySlots.Empty();
	// ItemClass가 Weapon이면 Weapon 슬롯만, 아니면 Inven 슬롯만
	const bool bIsWeapon = ItemClass->IsChildOf<ANAWeapon>();
    
	for (const auto& Pair : InventoryContents)
	{
		if (!Pair.Value.IsValid())
		{
			const FString SlotStr = Pair.Key.ToString();
			if (bIsWeapon ? SlotStr.StartsWith(TEXT("Weapon_")) : SlotStr.StartsWith(TEXT("Inven_")))
			{
				OutEmptySlots.Add(Pair.Key);
			}
		}
	}
}

int32 UNAInventoryComponent::ComputeDistributableAmount(UNAItemData* InputItem, const TArray<FName>& PartialSlots, const TArray<FName>& EmptySlots) const
{
	if (!InputItem) { return 0; }
	const int32 RequestedAmount = InputItem->GetQuantity();
	const int32 MaxInvHoldCount = InputItem->GetMaxInventoryHoldCount();

	// 1) Partial 슬롯에 이미 들어있는 수량
	int32 CurrTotalHold = 0;
	for (const FName& Slot : PartialSlots)
	{
		if (!InventoryContents[Slot].IsValid()) { continue; }
		CurrTotalHold += InventoryContents[Slot]->GetQuantity();
	}
	const int32 RemainingHold = MaxInvHoldCount == 0 ?
									RequestedAmount :
									FMath::Max(0, MaxInvHoldCount - CurrTotalHold);
	if (RemainingHold <= 0) { return 0; }

	// 2) Partial 슬롯들의 여유 공간 합산
	int32 PartialCapacity = 0;
	for (const FName& Slot : PartialSlots)
	{
		if (!InventoryContents[Slot].IsValid()) { continue; }
		PartialCapacity +=
			InventoryContents[Slot]->GetItemMaxSlotStackSize() - InventoryContents[Slot]->GetQuantity();
	}
	
	// 3) Empty 슬롯에서 만들 수 있는 신규 슬롯 여유 공간 합산
	int32 EmptyCapacity = 0;
	if (EmptySlots.Num() > 0)
	{
		const int32 MaxSlotStack = InputItem->GetItemMaxSlotStackSize();
		EmptyCapacity = EmptySlots.Num() * MaxSlotStack;
	}

	// 4) 최종으로 담을 수 있는 총 용량
	const int32 TotalSlotCapacity = PartialCapacity + EmptyCapacity;
	const int32 MaxAddableBySlot = FMath::Min(TotalSlotCapacity, RemainingHold);

	return FMath::Min(RequestedAmount, MaxAddableBySlot);
}

int32 UNAInventoryComponent::DistributeToSlots(UNAItemData* InputItem, int32 AmountToDistribute, const TArray<FName>& PartialSlots, const TArray<FName>& EmptySlots)
{
	 int32 Distributed = 0;

    // 1) Partial 슬롯 우선 채우기 (기존 슬롯을 최대치까지 채우며, 원본 수량만큼 차감)
    for (const FName& Slot : PartialSlots)
    {
        if (AmountToDistribute <= 0) { break; }
        UNAItemData* SlotItem = InventoryContents[Slot].Get();
        if (!SlotItem) { continue; }

        const int32 CurrQty   = SlotItem->GetQuantity();
        const int32 MaxSlot   = SlotItem->GetItemMaxSlotStackSize();
        const int32 CanAdd    = MaxSlot - CurrQty;
        if (CanAdd <= 0) { continue; }

        // 이 슬롯에 채울 수 있는 양
        const int32 ToAdd = FMath::Min(CanAdd, AmountToDistribute);

        // ─── 기존 슬롯에 수량 반영 ─────────────────────────────────
        SlotItem->SetQuantity(CurrQty + ToAdd);

        // ─── 원본 아이템 수량 차감 ─────────────────────────────────
        InputItem->SetQuantity(InputItem->GetQuantity() - ToAdd);

        AmountToDistribute -= ToAdd;
        Distributed       += ToAdd;
    }

    // 2) 남은 수량이 있으면 빈 슬롯에 채우기
    for (const FName& Slot : EmptySlots)
    {
        if (AmountToDistribute <= 0) { break; }
        if (InventoryContents[Slot].IsValid())
        {
            // 이미 다른 프로세스에서 채워졌거나, 슬롯에 데이터가 있는 경우 건너뜀
            continue;
        }

        // 현재 원본에 남은 수량
        const int32 RemainQty = InputItem->GetQuantity();
        if (RemainQty <= 0)
        {
            break;
        }

        // 한 슬롯에 채울 수 있는 최대치
        const int32 MaxSlot = InputItem->GetItemMaxSlotStackSize();

        // 기본적으로 이 슬롯에 채울 양
        int32 ToAdd = FMath::Min(MaxSlot, AmountToDistribute);

        // ─── 원본 수량 전부를 이 슬롯에 넣을 수 있다면, 복제 대신 원본을 이동 ───────────────────────
        if (ToAdd >= RemainQty)
        {
            // (1) 이 슬롯에 들어갈 양을 “원본 수량(RemainQty)”으로 조정
            ToAdd = RemainQty;

            // (2) 원본 데이터(InputItem)를 그대로 이 슬롯에 등록
            HandleAddNewItem(InputItem, Slot);

        	// ───── *중요* ─────  
        	// 이제 InputItem 객체는 “인벤토리 슬롯에 들어간 데이터”가 되므로, 
        	// InputItem->Quantity(==RemainQty) 값을 바꿀 필요가 없습니다.
        	// (절대로 InputItem->SetQuantity(0) 하지 마세요.)
        	
            Distributed       += ToAdd;
            AmountToDistribute -= ToAdd;

        	// 원본을 완전히 이동했으므로, 더 이상 빈 슬롯 순회를 하지 않고 루프 종료
            break;
        }
        // ─── 원본 수량 전부를 이 슬롯에 다 넣을 수 없다면, “복제본”을 만들어 슬롯에 일부 채우기 ──────────
        else
        {
            // (1) 원본 데이터(InputItem)의 복제본 생성
            UNAItemData* Duplicated = UNAItemEngineSubsystem::Get()->CreateItemDataCopy(InputItem);
            if (!Duplicated)
            {
                // 복제 실패 시, 이 슬롯도 채우지 않고 루프 종료
                break;
            }

            // (2) 복제본의 수량을 이 슬롯에 들어갈 양(ToAdd)으로 설정
            Duplicated->SetQuantity(ToAdd);

            // (3) 복제본을 슬롯에 등록
            HandleAddNewItem(Duplicated, Slot);

            // (4) 원본 수량에서ToAdd를 차감
            InputItem->SetQuantity(RemainQty - ToAdd);

            AmountToDistribute -= ToAdd;
            Distributed       += ToAdd;

            // 원본에 여전히 수량이 남아있으므로 루프는 계속
        }
    }

    return Distributed;
}

bool UNAInventoryComponent::IsValidForNonStackable(UNAItemData* InputItem, FString& OutFailReason) const
{
	if (!InputItem)
	{
		OutFailReason = TEXT("ItemToAdd was null.");
		return false;
	}
	// 1) 슬롯 1개에만 들어가는 아이템인지 확인
	const int32 MaxSlot = InputItem->GetItemMaxSlotStackSize();
	if (MaxSlot != 1)
	{
		OutFailReason = TEXT("비-스택형 아이템이 아닙니다.");
		return false;
	}
	// 2) 수량이 1인지
	if (InputItem->GetQuantity() != 1)
	{
		OutFailReason = TEXT("비-스택형 아이템의 수량은 1이어야 합니다.");
		return false;
	}
	// 3) 최대 소지 개수 검사
	const int32 MaxHold = InputItem->GetMaxInventoryHoldCount();
	if (MaxHold == 1 && HasItemOfClass(InputItem->GetItemActorClass()))
	{
		OutFailReason = TEXT("이미 동일한 아이템을 소지중입니다.");
		return false;
	}
	return true;
}

UNAItemData* UNAInventoryComponent::FindSameClassItem(const UClass* ItemClass) const
{
	if (ItemClass)
	{
		for (const auto& Pair : InventoryContents)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			if (Pair.Value->GetItemActorClass() == ItemClass)
			{
				return Pair.Value.Get();
			}
		}
	}
	return nullptr;
}

bool UNAInventoryComponent::FindSlotIDForItem(const UNAItemData* ItemToFind, FName& OutName) const
{
	if (ItemToFind && ItemToFind->GetOwningInventory() == this)
	{
		for (const auto& Pair : InventoryContents)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			if (Pair.Value.Get() == ItemToFind)
			{
				OutName = Pair.Key;
				return true;
			}
		}
	}
	return false;
}

bool UNAInventoryComponent::HandleAddNewItem(UNAItemData* NewItemToAdd, const FName& SlotID)
{
	if (SlotID.IsNone() || !InventoryContents.Contains(SlotID))
	{
		ensure(false);
		return false;
	}
	UNAInventoryGameInstanceSubsystem* InvenSubsys = UNAInventoryGameInstanceSubsystem::Get(GetWorld());
	if (!InvenSubsys)
	{
		ensure(false);
		return false;
	}
	
	InventoryContents[SlotID] = NewItemToAdd;
	NewItemToAdd->SetOwningInventory(this);
	InvenSubsys->AddItemToInventory(this, SlotID, NewItemToAdd);
	return true;
}

void UNAInventoryComponent::InitWidget()
{
	Super::InitWidget();

	if (IsValid(GetWidget()))
	{
		if (UNAInventoryWidget* InventoryWidget = Cast<UNAInventoryWidget>(GetWidget()))
		{
			const bool bSucceed = InventoryWidget->MapSlotIDAndUIButton(SlotButtons);
			//ensureAlwaysMsgf(bSucceed, TEXT("[UNAInventoryComponent::InitWidget]  인벤토리 슬롯 & UI 매핑 실패"));
		}
	}
}

UNAInventoryWidget* UNAInventoryComponent::GetInventoryWidget() const
{
	return Cast<UNAInventoryWidget>(GetWidget());
}

UButton* UNAInventoryComponent::GetSlotButton(const FName& SlotID) const
{
	if (SlotID.IsNone()
		|| !InventoryContents.Contains(SlotID)
		|| !SlotButtons.Contains(SlotID))
	{
		ensureAlwaysMsgf(false, TEXT("[UNAInventoryComponent::GetSlotButton]  유효하지 않은 슬롯ID: %s"), *SlotID.ToString());
		return nullptr;
	}

	return SlotButtons[SlotID].Get();
}

UButton* UNAInventoryComponent::GetSlotButton(const UNAItemData* ItemData) const
{
	FName SlotID = NAME_None;
	const bool bFoundResult = FindSlotIDForItem(ItemData, SlotID);
	if (!ItemData
		|| !bFoundResult
		|| !SlotButtons.Contains(SlotID))
	{
		ensureAlwaysMsgf(false, TEXT("[UNAInventoryComponent::GetSlotButton]  유효하지 않은 아이템 데이터: %s"),
			*ItemData->GetItemID().ToString());
		return nullptr;
	}

	return SlotButtons[SlotID].Get();
}

bool UNAInventoryComponent::IsInventoryWidgetVisible() const
{
	return IsVisible() && IsWidgetVisible();
}

void UNAInventoryComponent::ReleaseInventoryWidget()
{
	if (UUserWidget* InventoryWidget = GetWidget())
	{
		SetVisibility(true);
		InventoryWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UNAInventoryComponent::CollapseInventoryWidget()
{
	if (UUserWidget* InventoryWidget = GetWidget())
	{
		SetVisibility(false);
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

