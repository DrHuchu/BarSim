// Fill out your copyright notice in the Description page of Project Settings.


#include "OldPalCharacter.h"
#include "OldPalAnimInstance.h"
#include "OldPalFSM.h"
#include "OldPalOrderWidget.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AOldPalCharacter::AOldPalCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bodyComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("bodyComp"));
	bodyComp->SetupAttachment(GetMesh());

	ConstructorHelpers::FClassFinder<UAnimInstance> tempAnim(TEXT("/Script/Engine.AnimBlueprint'/Game/Jang/ABP_OldPalAnim.ABP_OldPalAnim_C'"));
	if(tempAnim.Succeeded())
	{
		bodyComp->SetAnimInstanceClass(tempAnim.Class);
	}
	
	oldPalFSM = CreateDefaultSubobject<UOldPalFSM>(TEXT("oldPalFSM"));

	oldPalWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("oldPalWidget"));
	oldPalWidget->SetupAttachment(GetCapsuleComponent());
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AOldPalCharacter::BeginPlay()
{
	Super::BeginPlay();

	oldPalAnim = Cast<UOldPalAnimInstance>(bodyComp->GetAnimInstance());

	oldPal_UI = Cast<UOldPalOrderWidget>(oldPalWidget->GetUserWidgetObject());

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), enterSound, GetActorLocation(), GetActorRotation());
}

// Called every frame
void AOldPalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(oldPalFSM->player != nullptr)
	{
		FVector tar = oldPalWidget->GetComponentLocation();
		FVector direction = oldPalFSM->player->VRReplicatedCamera->GetComponentLocation();
		FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(tar,direction);
		oldPalWidget->SetWorldRotation(FMath::Lerp(oldPalWidget->GetComponentRotation(), lookAtRotation, 0.1f));
	}
}

// Called to bind functionality to input
void AOldPalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

