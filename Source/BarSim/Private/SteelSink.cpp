// Fill out your copyright notice in the Description page of Project Settings.


#include "SteelSink.h"

// Sets default values
ASteelSink::ASteelSink()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	sinkComp=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("sinkComp"));
	SetRootComponent(sinkComp);
}

// Called when the game starts or when spawned
void ASteelSink::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASteelSink::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

