// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DropBase.generated.h"

UCLASS()
class BARSIM_API ADropBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADropBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Drop")
	class USphereComponent* sphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Drop")
	float dropMass;

	UPROPERTY(EditAnywhere, Category = "Drop")
	FString name;

	UFUNCTION()
	void DropDestroy();

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	class ACupBase* cup;

	UFUNCTION()
	void DropDestroyDelay();
};
