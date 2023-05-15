// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CupWidget.generated.h"

/**
 * 
 */
UCLASS()
class BARSIM_API UCupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Name1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Name2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Name3;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Name4;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Contents1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Contents2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Contents3;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Contents4;
	
};
