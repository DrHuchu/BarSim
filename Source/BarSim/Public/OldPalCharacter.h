// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OldPalCharacter.generated.h"

UCLASS()
class BARSIM_API AOldPalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOldPalCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=FSM)
	class UOldPalFSM* oldPalFSM;

	UPROPERTY(EditAnywhere)
	class UOldPalAnimInstance* oldPalAnim;

	UPROPERTY(EditAnywhere, Category=Widget)
	class UWidgetComponent* oldPalWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* bodyComp;

	UPROPERTY()
	class UOldPalOrderWidget* oldPal_UI;

	UPROPERTY(EditAnywhere, Category=Sound)
	class USoundBase* enterSound;

	UPROPERTY(EditAnywhere, Category=Sound)
	class USoundBase* cashSound;
	
	UPROPERTY()
	class ACupBase* cup;
	
	// 캐릭터 컵 오버랩 바인드 함수
	UFUNCTION()
	void BindOldPalOverlap();

	UFUNCTION()
	void DetachCup();

	// 음성 관련
	UPROPERTY(EditAnywhere, Category = "Voice")
	class UAudioComponent* oldPalVoice1st;

	UPROPERTY(EditAnywhere, Category = "Voice")
	class UAudioComponent* oldPalVoice3rd;

	UPROPERTY(EditAnywhere, Category = "Voice")
	class UAudioComponent* playerVoice1st;

	UPROPERTY(EditAnywhere, Category = "Voice")
	class UAudioComponent* playerVoice3rd;
};
