// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CupBase.h"
#include "MixingGlass.generated.h"

/**
 * 
 */
UCLASS()
class BARSIM_API AMixingGlass : public ACupBase
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(VisibleAnywhere, Category = "MixingGlass")
	FVector upVector = FVector(0,0,1);

	UPROPERTY(EditAnywhere, Category = "MixingGlass")
	class UNiagaraSystem* streamFX;

	UPROPERTY(VisibleAnywhere, Category = "MixingGlass")
	class UNiagaraComponent* waterStream;

	bool bStreamOn = false;

	UPROPERTY(EditAnywhere, Category = "MixingGlass")
	TSubclassOf<class ADropBase> streamDrop;
};
