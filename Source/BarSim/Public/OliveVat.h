// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grippables/GrippableActor.h"
#include "OliveVat.generated.h"

UCLASS()
class BARSIM_API AOliveVat : public AGrippableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOliveVat(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Olive")
	class UStaticMeshComponent* oliveVatComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	bool isDropSoundEnabled = false;

	UFUNCTION()
	void SetSimulateAndTickDisable();

	UPROPERTY()
	FTimerHandle simHandle;

	UFUNCTION()
	void SimulateAndTickDisable();

};
