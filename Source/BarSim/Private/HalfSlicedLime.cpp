// Fill out your copyright notice in the Description page of Project Settings.


#include "HalfSlicedLime.h"

AHalfSlicedLime::AHalfSlicedLime(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	limeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("limeMesh"));
	limeMesh->SetupAttachment(RootComponent);
	
}