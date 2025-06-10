// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NAPlayerState.generated.h"

class UNAVitalCheckComponent;
/**
 * 
 */
UCLASS()
class ARPG_API ANAPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	// 현재 체력
	float GetHealth() const;
	
	// 최대 체력
	int32 GetMaxHealth() const;

	// 캐릭터의 VitalCheckComponent 래퍼 함수
	// 캐릭터가 여럿이 될 경우나 바뀔 경우를 위해 PlayerState로 빼놓음
	// ============================
	// 생존 상태 반환
	bool IsAlive() const;
	
	// 녹다운 상태 반환
	bool IsKnockDown() const;

	// Zoom 상태 반환
	bool IsZoom() const;

	// ============================

	// 빙의한 Pawn의 에셋 이름 반환
	FName GetPossessAssetName() const { return PossessAssetName; }

	// 빙의한 Pawn의 에셋 이름 설정
	void SetPossessAssetName(const FName& AssetName);

protected:

	// 서버에서 AssetName이 바뀐 경우 클라이언트 사이드에서 에셋 업데이트
	UFUNCTION()
	void OnRep_PossessAssetName();

	// Pawn의 AssetName을 수정하는 함수
	UFUNCTION()
	void UpdatePossessAssetByName();

	virtual void BeginPlay() override;

	virtual void PostNetInit() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어 또는 NPC의 에셋
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_PossessAssetName)
	FName PossessAssetName = NAME_None;
	
};
