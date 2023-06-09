// Fill out your copyright notice in the Description page of Project Settings.


#include "BarPlayer.h"

#include "BarFridge.h"
#include "BarSpoon.h"
#include "BottleBase.h"
#include "Coaster.h"
#include "CupBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "HuchuTong.h"
#include "GripMotionControllerComponent.h"
#include "IceCube.h"
#include "Opener.h"
#include "Tablet.h"
#include "VRBaseCharacter.h"
#include "XRMotionControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values
ABarPlayer::ABarPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCapsuleRadius(16.0f);
	
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);
	LeftHand->SetTrackingMotionSource(FName("Left"));
	
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(RootComponent);
	RightHand->SetTrackingMotionSource(FName("Right"));

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHand);
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHand);


	LeftAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftAim"));
	LeftAim->SetupAttachment(RootComponent);
	LeftAim->SetTrackingMotionSource(FName("LeftAim"));
	RightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightAim"));
	RightAim->SetupAttachment(RootComponent);
	RightAim->SetTrackingMotionSource(FName("RightAim"));

	LeftMotionController = CreateDefaultSubobject<UGripMotionControllerComponent>(TEXT("LeftGripMotionController"));
	if (IsValid(LeftMotionController))
	{
		LeftMotionController->SetupAttachment(LeftHand);
		//LeftMotionController->MotionSource = FXRMotionControllerBase::LeftHandSourceId;
		LeftMotionController->SetTrackingMotionSource(FXRMotionControllerBase::LeftHandSourceId);
		//LeftMotionController->Hand = EControllerHand::Left;
		LeftMotionController->bOffsetByHMD = false;
		//LeftMotionController->bUpdateInCharacterMovement = true;
		// Keep the controllers ticking after movement
	}

	RightMotionController = CreateDefaultSubobject<UGripMotionControllerComponent>(TEXT("RightGripMotionController"));
	if (IsValid(RightMotionController))
	{
		RightMotionController->SetupAttachment(RightHand);
		//RightMotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
		RightMotionController->SetTrackingMotionSource(FXRMotionControllerBase::RightHandSourceId);
		//RightMotionController->Hand = EControllerHand::Right;
		RightMotionController->bOffsetByHMD = false;
		//RightMotionController->bUpdateInCharacterMovement = true;
		// Keep the controllers ticking after movement
	}
	
	widgetInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("widgetInteractionComp"));
	widgetInteractionComp->SetupAttachment(RightAim);

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/KHJContents/Meshes/PlayerHandsMesh/HTAB/meshes/realistic_hand/realistic_hand_l_ue4.realistic_hand_l_ue4'"));
	if (TempMesh.Succeeded())
	{
		LeftHandMesh->SetSkeletalMesh(TempMesh.Object);
		LeftHandMesh->SetRelativeLocation(FVector(-2.9f, -3.5f, 4.5f));
		LeftHandMesh->SetRelativeRotation(FRotator(-25, -180, 90));
	}

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh2(TEXT("/Script/Engine.SkeletalMesh'/Game/KHJContents/Meshes/PlayerHandsMesh/HTAB/meshes/realistic_hand/realistic_hand_r_ue4.realistic_hand_r_ue4'"));
	if (TempMesh2.Succeeded())
	{
		RightHandMesh->SetSkeletalMesh(TempMesh2.Object);
		RightHandMesh->SetRelativeLocation(FVector(-2.9f, 3.5f, 4.5f));
		RightHandMesh->SetRelativeRotation(FRotator(25, 0, 90));
	}

	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	FPSCamera->SetupAttachment(RootComponent);
	FPSCamera->SetRelativeLocation(FVector(-15, 0, 30));

	widgetInteractionComp->SetAutoActivate(true);

}

// Called when the game starts or when spawned
void ABarPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input 
	auto PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		// LocalPlayer
		auto localPlayer = PC->GetLocalPlayer();
		auto subSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(localPlayer);
		if (subSystem)
		{
			subSystem->AddMappingContext(IMC_Default, 0);
			subSystem->AddMappingContext(IMC_Hand, 0);
		}
	}
	// HMD가 연결되어 있지 않다면,
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		RightAim->SetRelativeLocation(FVector(20, 20, 0));
		RightHand->SetRelativeLocation(FVector(20, 20, 0));
		FPSCamera->bUsePawnControlRotation = true;
		bUseControllerRotationYaw=true;
		GrabRange = 45.0f;
		FPSCamera->AddRelativeLocation(FVector(0, 0, 22));
	}
	// HMD가 연결되어 있다면,
	else
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		FPSCamera->bUsePawnControlRotation = false;
		bUseControllerRotationYaw=false;
		GrabRange=15.0f;
	}
	widgetInteractionComp->DebugSphereLineThickness=0.5f;
	widgetInteractionComp->DebugLineThickness=0.3f;
	widgetInteractionComp->DebugColor=FColor::Red;
	widgetInteractionComp->bEnableHitTesting=true;
	widgetInteractionComp->bShowDebug=false;	
	widgetInteractionComp->InteractionDistance=50.0f;
	GetCharacterMovement()->bEnablePhysicsInteraction=false;
}

// Called every frame
void ABarPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		RightHand->SetRelativeRotation(FPSCamera->GetRelativeRotation());
		RightAim->SetRelativeRotation(FPSCamera->GetRelativeRotation());
	}
	
	// 잡은 대상을 던질 위치값 실시간 업데이트
	Grabbing();

	// Tongs가 nullptr이 아니면서
	if(huchuTong!=nullptr)
	{
		// 오른손에 Tongs를 쥐고 있다면
		if(isGrabbingTongsRight)
		{
			// Oculus Trigger Input Value에 따른 Tongs Rotation 제어 Tick 활성화가 되어있다면
			if(isTongsTickEnabled == true)
			{
				huchuTong->tongRight->SetRelativeRotation(FRotator(fingerPressedActionValue*15, 0, 0));
			}
			
		}
	}
	// Tongs가 nullptr이 아니면서
	if(huchuTongL!=nullptr)
	{
		// 왼손에 Tongs를 쥐고 있다면
		if(isGrabbingTongsLeft)
		{
			// Oculus Trigger Input Value에 따른 Tongs Rotation 제어 Tick 왼쪽 활성화가 되어있다면
			if(isTongsTickEnabledL == true)
			{
				huchuTongL->tongRight->SetRelativeRotation(FRotator(fingerPressedActionValueLeft*15, 0, 0));
			}
			
		}
	}
	// FridgeDoor가 nullptr이 아니면서
	if(barFridge!=nullptr)
	{
		// 오른손에 Fridge Door를 잡고 있다면
		if(isGrabbingFridgeDoorRight)
		{
			auto doorYaw = FMath::Clamp(GetDoorAngle()*1.8-27, 20, 150);
			auto doorYawR = FMath::Clamp(GetDoorAngleR(), 0, 90);
			
			//UE_LOG(LogTemp, Warning, TEXT("%f"), doorYaw)
			GrabbedObjectRight->SetRelativeRotation(FRotator(0, -((doorYaw/2)-(doorYawR/1.5))+130, 0));		}
	}
	// FridgeDoorL가 nullptr이 아니면서
	if(barFridgeL!=nullptr)
	{
		// 왼손에 Fridge Door를 잡고 있다면
		if(isGrabbingFridgeDoorLeft)
		{
			auto doorYawLeft = FMath::Clamp(GetDoorAngleLeft()*1.8-27, 20, 150);
			auto doorYawLeftR = FMath::Clamp(GetDoorAngleLeftR(), 0, 90);
			//UE_LOG(LogTemp, Warning, TEXT("%f"), doorYawLeft)
			GrabbedObjectLeft->SetRelativeRotation(FRotator(0, -((doorYawLeft/2)-(doorYawLeftR/1.5))+130, 0));
		}
	}
	// Cup이 nullptr이 아니면서, 오른손에 Cup을 쥐고 있다면
	if(isGrabbingCupRight&&cup!=nullptr)
	{
		// cup의 contents가 0보다 크다면
		if(cup->contents>0)
		{
			// 컵을 쥔 순간의 Rotation 값을 유지한다.
			RightHand->SetRelativeRotation(initHandRot);
		}
	
	}
	// Cup이 nullptr이 아니면서, 왼손에 Cup을 쥐고 있다면
	if(isGrabbingCupLeft&&cupL!=nullptr)
	{
		//cup 의 contents가 0보다 크다면,
		if(cupL->contents>0)
		{
			// 컵을 쥔 순간의 Rotation 값을 유지한다.
			LeftHand->SetRelativeRotation(initHandRotL);
		}

	}
}

// Called to bind functionality to input
void ABarPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto InputSystem = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (InputSystem)
	{
		//Binding
		InputSystem->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ABarPlayer::Move);
		InputSystem->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ABarPlayer::Turn);
		InputSystem->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ABarPlayer::Jump);
		InputSystem->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ABarPlayer::JumpEnd);
		InputSystem->BindAction(IA_Grab_Left, ETriggerEvent::Started, this, &ABarPlayer::TryGrabLeft);
		InputSystem->BindAction(IA_Grab_Left, ETriggerEvent::Completed, this, &ABarPlayer::UnTryGrabLeft);
		InputSystem->BindAction(IA_Grab_Right, ETriggerEvent::Started, this, &ABarPlayer::TryGrabRight);
		InputSystem->BindAction(IA_Grab_Right, ETriggerEvent::Completed, this, &ABarPlayer::UnTryGrabRight);
		InputSystem->BindAction(IA_Fire, ETriggerEvent::Started, this, &ABarPlayer::Fire);
		InputSystem->BindAction(IA_FireLeft, ETriggerEvent::Started, this, &ABarPlayer::FireLeft);
		InputSystem->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ABarPlayer::FireReleased);
		InputSystem->BindAction(IA_FireLeft, ETriggerEvent::Completed, this, &ABarPlayer::FireReleasedLeft);
	}
	
}

void ABarPlayer::Move(const FInputActionValue& Values)
{
	FVector2D Axis = Values.Get<FVector2D>();
	AddMovementInput(FPSCamera->GetForwardVector(), Axis.Y);
	AddMovementInput(FPSCamera->GetRightVector(), Axis.X);
}

void ABarPlayer::Turn(const FInputActionValue& Values)
{
	FVector2D Axis = Values.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ABarPlayer::Jump()
{
	ACharacter::Jump();
}

void ABarPlayer::JumpEnd()
{
	ACharacter::StopJumping();
}


void ABarPlayer::TongsMovementExec()
{
	IsTongsMovementFinished=true;
	
}

void ABarPlayer::TongsReleaseMovementExec()
{
	IsTongsReleaseMovementFinished=true;
}

float ABarPlayer::GetDoorAngle()
{
	//bool crossBoolean = false;
	auto initDoorLocation= GrabbedObjectRight->GetComponentLocation();
	auto rightHandLoc = RightHand->GetComponentLocation();
	float doorDist = (initDoorLocation-rightHandLoc).X;
	//auto doorPivotLoc = GrabbedObjectRight->GetComponentLocation();
	//doorVec.Normalize();
	//auto doorVecDot = FVector::DotProduct(doorVec, initDoorDirection);
	//auto doorACOSd = UKismetMathLibrary::DegAcos(doorVecDot);
	//auto doorVecCross = UKismetMathLibrary::Cross_VectorVector(doorVec, initDoorDirection).Z;
	//doorVecCross <0 ? crossBoolean = true : crossBoolean = false;
	//auto selectFl = UKismetMathLibrary::SelectFloat(-1.0f, 1.0f, crossBoolean);
	
	return doorDist;
}

float ABarPlayer::GetDoorAngleLeft()
{
	//bool crossBoolean = false;
	auto initDoorLocation= GrabbedObjectLeft->GetComponentLocation();
	auto leftHandLoc = LeftHand->GetComponentLocation();
	float doorDist = (initDoorLocation-leftHandLoc).X;	
	//auto doorPivotLoc = GrabbedObjectRight->GetComponentLocation();
	//doorVec.Normalize();
	//auto doorVecDot = FVector::DotProduct(doorVec, initDoorDirection);
	//auto doorACOSd = UKismetMathLibrary::DegAcos(doorVecDot);
	//auto doorVecCross = UKismetMathLibrary::Cross_VectorVector(doorVec, initDoorDirection).Z;
	//doorVecCross <0 ? crossBoolean = true : crossBoolean = false;
	//auto selectFl = UKismetMathLibrary::SelectFloat(-1.0f, 1.0f, crossBoolean);
	
	return doorDist;
}

float ABarPlayer::GetDoorAngleR()
{
	auto initDoorLocation= GrabbedObjectRight->GetComponentLocation();
	auto rightHandLoc = RightHand->GetComponentLocation();
	float doorDistR = (initDoorLocation-rightHandLoc).Y;
	return  doorDistR;
}

float ABarPlayer::GetDoorAngleLeftR()
{
	auto initDoorLocation= GrabbedObjectLeft->GetComponentLocation();
	auto leftHandLoc = LeftHand->GetComponentLocation();
	float doorDistR = (initDoorLocation-leftHandLoc).Y;
	return  doorDistR;
}


void ABarPlayer::TryGrabLeft()
{
	// 중심점
	FVector Center = LeftHandMesh->GetComponentLocation();
	// 충돌체크(구충돌)
	// 충돌한 물체를 기억할 배열
	TArray<FOverlapResult> HitObj;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	params.AddIgnoredComponent(RightHand);
	params.AddIgnoredComponent(RightHandMesh);	
	params.AddIgnoredComponent(LeftHand);
	params.AddIgnoredComponent(LeftHandMesh);
	//DrawDebugSphere(GetWorld(), Center, GrabRange, 30, FColor::Red, false, 0.3, 0, 0.1);
	bool bHit = GetWorld()->OverlapMultiByChannel(HitObj, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GrabRange), params);
	if (bHit == false)
	{
		return;
	}
	// 가장 가까운 물체를 잡도록 하자
	// 가장 가까운 물체 인덱스
	int32 Closest = 0;
	for (int i = 0; i < HitObj.Num(); ++i)
	{
		// 1. 물리기능이 활성화 되어 있는 대상만 판단
		if (HitObj[i].GetComponent()->IsSimulatingPhysics() == false)
		{
			continue;
		}
		// 잡기에 성공했다
		IsGrabbedLeft = true;
		// 2.. 현재 손과 가장 가까운 대상과 이번에 검출할 대상과 더 가까운 대상이 있다면		
		// 필요속성 : 현재 가장 가까운 대상과 손과의 거리
		float ClosestDist = FVector::Dist(HitObj[Closest].GetActor()->GetActorLocation(), Center);
		// 필요속성 : 이번에 검출할 대상과 손과의 거리
		float NextDist = FVector::Dist(HitObj[i].GetActor()->GetActorLocation(), Center);

		// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
		if (NextDist < ClosestDist)
		{

			// 가장 가까운 대상으로 변경하기
			Closest = i;
		}
	}

	// 잡기에 성공했다면
	if (IsGrabbedLeft&&HitObj[Closest].GetComponent()->IsSimulatingPhysics() == true)
	{
		// 물체 물리기능 비활성화
		GrabbedActorLeft=HitObj[Closest].GetComponent()->GetAttachmentRootActor();
		GrabbedObjectLeft = HitObj[Closest].GetComponent();
		GrabbedObjectLeft->SetSimulatePhysics(false);
		// 손에 붙여주자
		// Left Grabbed Actor Casting
		huchuTongL=Cast<AHuchuTong>(GrabbedActorLeft);
		bottleL = Cast<ABottleBase>(GrabbedActorLeft);
		tabletL = Cast<ATablet>(GrabbedActorLeft);
		barFridgeL=Cast<ABarFridge>(GrabbedActorLeft);
		openerL=Cast<AOpener>(GrabbedActorLeft);
		coasterL=Cast<ACoaster>(GrabbedActorLeft);
		cupL=Cast<ACupBase>(GrabbedActorLeft);
		barSpoonL=Cast<ABarSpoon>(GrabbedActorLeft);
		// 잡은 대상이 Tongs라면
		if(GrabbedActorLeft==huchuTongL&&huchuTongL!=nullptr)
		{
			isGrabbingTongsLeft=true;
			GrabbedObjectWithTongsLeft = nullptr;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("TongsSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab huchu on Left"))
		}
		// 잡은 대상이 Bottle 이라면
		else if(GrabbedActorLeft == bottleL&&bottleL!=nullptr)
		{
			isGrabbingBottleLeft = true;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("BottleSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab bottle on Left"))
		}
		// 잡은 대상이 Tablet 이라면
		else if(GrabbedActorLeft==tabletL&&tabletL!=nullptr)
		{
			isGrabbingTabletLeft=true;
			widgetInteractionComp->bShowDebug=true;
			//widgetInteractionComp->bEnableHitTesting=true;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("TabletSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			//GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab tablet on Left"))
		}
		// 잡은 대상이 Fridge Door 라면
		else if(GrabbedActorLeft==barFridgeL&&barFridgeL!=nullptr)
		{
			isGrabbingFridgeDoorLeft=true;			
			LeftHandMesh->SetVisibility(false);
			//GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab Fridge Door on Left"))

		}
		// 잡은 대상이 Opener 이라면
		else if(GrabbedActorLeft==openerL&&openerL!=nullptr)
		{
			isGrabbingOpenerLeft=true;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("OpenerSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab opener on Left"))			
		}
		// 잡은 대상이 Coaster이라면
		else if(GrabbedActorLeft==coasterL&&coasterL!=nullptr)
		{
			isGrabbingCoasterLeft=true;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("OpenerSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab coaster on Left"))			
		}
		// 잡은 대상이 CupBase라면
		else if(GrabbedActorLeft==cupL&&cupL!=nullptr)
		{
			// Cup을 쥔 순간의 Hand Rotation 값 저장
			initHandRotL=LeftHand->GetRelativeRotation();
			isGrabbingCupLeft=true;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("CupSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab cup on Left"))			
		}
		// 잡은 대상이 BarSpoon이라면
		else if(GrabbedActorLeft==barSpoonL&&barSpoonL!=nullptr)
		{
			isGrabbingBarSpoonLeft=true;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("SpoonSocketLeft"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab barspoon on Left"))	
		}
		else
		{
			if(GrabbedObjectLeft->IsSimulatingPhysics()==false)
			{
				GrabbedObjectLeft->SetSimulatePhysics(true);
				IsGrabbedLeft=false;
				GrabbedObjectLeft=nullptr;
				GrabbedActorLeft=nullptr;
			}
			//GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("CompGrabSocket"),EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);

		}
		HitObj.Reset();
	}
	else
	{
		return;
	}
	
}

void ABarPlayer::TryGrabRight()
{
	// 중심점
	FVector Center = RightHandMesh->GetComponentLocation();
	// 충돌체크(구충돌)
	// 충돌한 물체를 기억할 배열
	TArray<FOverlapResult> HitObj;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	params.AddIgnoredComponent(RightHand);
	params.AddIgnoredComponent(RightHandMesh);	
	params.AddIgnoredComponent(LeftHand);
	params.AddIgnoredComponent(LeftHandMesh);
	//DrawDebugSphere(GetWorld(), Center, GrabRange, 30, FColor::Red, false, 0.3, 0, 0.1);
	bool bHit = GetWorld()->OverlapMultiByChannel(HitObj, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GrabRange), params);
	if (bHit == false)
	{
		return;
	}
	// 가장 가까운 물체를 잡도록 하자
	// 가장 가까운 물체 인덱스
	int32 Closest = 0;
	for (int i = 0; i < HitObj.Num(); ++i)
	{
		// 1. 물리기능이 활성화 되어 있는 대상만 판단
		if (HitObj[i].GetComponent()->IsSimulatingPhysics() == false)
		{
			continue;
		}
		// 잡기에 성공했다
		IsGrabbedRight = true;
		// 2.. 현재 손과 가장 가까운 대상과 이번에 검출할 대상과 더 가까운 대상이 있다면		
		// 필요속성 : 현재 가장 가까운 대상과 손과의 거리
		float ClosestDist = FVector::Dist(HitObj[Closest].GetActor()->GetActorLocation(), Center);
		// 필요속성 : 이번에 검출할 대상과 손과의 거리
		float NextDist = FVector::Dist(HitObj[i].GetActor()->GetActorLocation(), Center);

		// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
		if (NextDist < ClosestDist)
		{

			// 가장 가까운 대상으로 변경하기
			Closest = i;
		}
	}

	// 잡기에 성공했다면
	if (IsGrabbedRight&&HitObj[Closest].GetComponent()->IsSimulatingPhysics() == true)
	{
		// 물체 물리기능 비활성화
		GrabbedActorRight=HitObj[Closest].GetComponent()->GetAttachmentRootActor();
		GrabbedObjectRight = HitObj[Closest].GetComponent();
		GrabbedObjectRight->SetSimulatePhysics(false);
		// 손에 붙여주자
		// Right Grabbed Actor Casting
		huchuTong=Cast<AHuchuTong>(GrabbedActorRight);
		bottle = Cast<ABottleBase>(GrabbedActorRight);
		tablet = Cast<ATablet>(GrabbedActorRight);
		barFridge=Cast<ABarFridge>(GrabbedActorRight);
		opener=Cast<AOpener>(GrabbedActorRight);
		coaster=Cast<ACoaster>(GrabbedActorRight);
		cup=Cast<ACupBase>(GrabbedActorRight);
		barSpoon=Cast<ABarSpoon>(GrabbedActorRight);
		// 잡은 대상이 Tongs라면
		if(GrabbedActorRight==huchuTong&&huchuTong!=nullptr)
		{
			isGrabbingTongsRight=true;
			GrabbedObjectWithTongsRight = nullptr;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("TongsSocketRight"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,false);
			RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab huchu on Right"))
		}
		// 잡은 대상이 Bottle 이라면
		else if(GrabbedActorRight == bottle&&bottle!=nullptr)
		{
			isGrabbingBottleRight = true;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("BottleSocket"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,false);
			//RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);
			FinalizeAttach();

			UE_LOG(LogTemp, Warning, TEXT("grab bottle on Right"))
		}
		// 잡은 대상이 Tablet 이라면
		else if(GrabbedActorRight==tablet&&tablet!=nullptr)
		{
			isGrabbingTabletRight=true;
			widgetInteractionComp->bShowDebug=true;
			//widgetInteractionComp->bEnableHitTesting=true;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("TabletSocket"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,false);
			//RightHandMesh->SetVisibility(false);
			//GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab tablet on Right"))
		}
		// 잡은 대상이 Fridge Door 라면
		else if(GrabbedActorRight==barFridge&&barFridge!=nullptr)
		{
			isGrabbingFridgeDoorRight=true;			
			RightHandMesh->SetVisibility(false);
			//GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab Fridge Door on Right"))

		}
		// 잡은 대상이 Opener 이라면
		else if(GrabbedActorRight==opener&&opener!=nullptr)
		{
			isGrabbingOpenerRight=true;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("OpenerSocket"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab opener on Right"))			
		}
		// 잡은 대상이 Coaster이라면
		else if(GrabbedActorRight==coaster&&coaster!=nullptr)
		{
			isGrabbingCoasterRight=true;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("OpenerSocket"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab coaster on Right"))			
		}
		// 잡은 대상이 Cup이라면
		else if(GrabbedActorRight==cup&&cup!=nullptr)
		{
			// Cup을 쥔 순간의 Hand Rotation 값 저장
			initHandRot=RightHand->GetRelativeRotation();
			isGrabbingCupRight=true;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("CupSocket"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab cup on Right"))			
		}
		// 잡은 대상이 BarSpoon이라면
		else if(GrabbedActorRight==barSpoon&&barSpoon!=nullptr)
		{
			isGrabbingBarSpoonRight=true;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("SpoonSocket"),EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);
			RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Warning, TEXT("grab barspoon on Right"))			
		}
		else
		{
			if(GrabbedObjectRight->IsSimulatingPhysics()==false)
			{
				IsGrabbedRight=false;
				GrabbedObjectRight=nullptr;
				GrabbedActorRight=nullptr;
				FinalizeAttach();
			}
			//GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("CompGrabSocket"),EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);

		}
		HitObj.Reset();
	}
	else
	{
		return;
	}
}
void ABarPlayer::UnTryGrabLeft()
{
	// 왼손에 쥐고 있는 것이 없었다면,
	if (IsGrabbedLeft == false)
	{
		return;
	}
	// 왼손에 Tongs를 잡고 있었다면
	if(isGrabbingTongsLeft)
	{
		isGrabbingTongsLeft=false;
		IsGrabbedLeft = false;
		// Tongs에 잡혀 있는 대상이 있었다면
		if(isGrabbingWithTongsLeft)
		{
			UE_LOG(LogTemp, Warning, TEXT("Something was on Left tongs"))
			FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;
			FLatentActionInfo LatentInfoL;
			LatentInfoL.CallbackTarget = this;
			auto tongCompRef = huchuTongL->tongRight;
			auto tongCompRefL=huchuTongL->tongLeft;
			UKismetSystemLibrary::MoveComponentTo(tongCompRef, tongCompRef->GetRelativeLocation(), tongCompRef->GetRelativeRotation()+FRotator(-(grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfo);
			UKismetSystemLibrary::MoveComponentTo(tongCompRefL, tongCompRefL->GetRelativeLocation(), tongCompRefL->GetRelativeRotation()+FRotator((grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfoL);
			isTongsTickEnabledL = true;
			grabbingObjectSizeL = 0;
			// 1. 잡지않은 상태로 전환
			isGrabbingWithTongsLeft = false;
			// 2. 손에서 떼어내기
			GrabbedObjectWithTongsLeft->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			// 3. 물리기능 활성화
			GrabbedObjectWithTongsLeft->SetSimulatePhysics(true);
			// 4. 충돌기능 활성화
			GrabbedObjectWithTongsLeft->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GrabbedObjectWithTongsLeft = nullptr;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Nothing was on Left tongs"))
		}
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		GrabbedObjectLeft->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepWorld,EDetachmentRule::KeepWorld,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("release Left huchu"))
	}
	// 왼손에 Bottle을 쥐고 있었다면
	else if(isGrabbingBottleLeft)
	{
		isGrabbingBottleLeft=false;
		IsGrabbedLeft = false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Right Bottle"))
	}
	// 왼손에 Tablet을 쥐고 있었다면
	else if(isGrabbingTabletLeft)
	{
		isGrabbingTabletLeft=false;
		IsGrabbedLeft = false;
		widgetInteractionComp->bShowDebug=false;
		//widgetInteractionComp->bEnableHitTesting=false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		//GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		//GrabbedActorLeft->SetActorEnableCollision(true);
		//GrabbedObjectLeft->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);		
		//GrabbedActorLeft->K2_DetachFromActor(EDetachmentRule::KeepWorld,EDetachmentRule::KeepWorld,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft = nullptr;
		LeftHandMesh->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("release Left Tablet"))
	}
	// 왼손에 Fridge Door를 쥐고 있었다면
	else if(isGrabbingFridgeDoorLeft)
	{
		isGrabbingFridgeDoorLeft=false;
		IsGrabbedLeft = false;
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass()*50);
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);
	}
	// 왼손에 Opener를 쥐고 있었다면
	else if(isGrabbingOpenerLeft)
	{
		isGrabbingOpenerLeft=false;
		IsGrabbedLeft = false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass());
		float Angle;
		FVector Axis;
		DeltaRotation.ToAxisAndAngle(Axis, Angle);
		float dt = GetWorld()->DeltaTimeSeconds;
		FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Left Opener"))
	}
	// 왼손에 Coaster를 쥐고 있었다면
	else if(isGrabbingCoasterLeft)
	{
		isGrabbingCoasterLeft=false;
		IsGrabbedLeft = false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass());
		float Angle;
		FVector Axis;
		DeltaRotation.ToAxisAndAngle(Axis, Angle);
		float dt = GetWorld()->DeltaTimeSeconds;
		FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Left Coaster"))
	}
	// 왼손에 Cup을 쥐고 있었다면
	else if(isGrabbingCupLeft)
	{
		isGrabbingCupLeft=false;
		IsGrabbedLeft = false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		//GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Left Cup"))
	}
	// 왼손에 BarSpoon을 쥐고 있었다면
	else if(isGrabbingBarSpoonLeft)
	{
		isGrabbingBarSpoonLeft=false;
		IsGrabbedLeft = false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		//GrabbedObjectLeft->AddForce(ThrowDirection * ThrowPower * GrabbedObjectLeft->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
		GrabbedActorLeft=nullptr;
		LeftHandMesh->SetVisibility(true);
	}
	else
	{
		// 잡지않은 상태로 전환
		IsGrabbedLeft = false;
	}
	
}

void ABarPlayer::UnTryGrabRight()
{
	// 오른손에 쥐고 있는 것이 없었다면,
	if (IsGrabbedRight==false)
	{
		return;
	}
	// 오른손에 Tongs를 잡고 있었다면
	if(isGrabbingTongsRight)
	{		
		// Tongs에 잡혀 있는 대상이 있었다면
		if(isGrabbingWithTongsRight)
		{
			UE_LOG(LogTemp, Warning, TEXT("Something was on Right tongs"))
			IsTongsReleaseMovementFinished=false;
			FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;
			FLatentActionInfo LatentInfoL;
			LatentInfoL.CallbackTarget = this;
			auto tongCompRef = huchuTong->tongRight;
			auto tongCompRefL=huchuTong->tongLeft;
			UKismetSystemLibrary::MoveComponentTo(tongCompRef, tongCompRef->GetRelativeLocation(), tongCompRef->GetRelativeRotation()+FRotator(-(grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfo);
			UKismetSystemLibrary::MoveComponentTo(tongCompRefL, tongCompRefL->GetRelativeLocation(), tongCompRefL->GetRelativeRotation()+FRotator((grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfoL);
			isTongsTickEnabled = true;
			grabbingObjectSize = 0;
			// 1. 잡지않은 상태로 전환
			isGrabbingWithTongsRight = false;
			// 2. 손에서 떼어내기
			GrabbedObjectWithTongsRight->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			// 3. 물리기능 활성화
			GrabbedObjectWithTongsRight->SetSimulatePhysics(true);
			// 4. 충돌기능 활성화
			GrabbedObjectWithTongsRight->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GrabbedObjectWithTongsRight = nullptr;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Nothing was on Right tongs"))
		}
		isGrabbingTongsRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("release Right huchu"))
	}
	// 오른손에 Bottle을 쥐고 있었다면
	else if(isGrabbingBottleRight)
	{
		isGrabbingBottleRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		//RightHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Right Bottle"))
	}
	// 오른손에 Tablet을 쥐고 있었다면
	else if(isGrabbingTabletRight)
	{
		isGrabbingTabletRight=false;
		widgetInteractionComp->bShowDebug=false;
		//widgetInteractionComp->bEnableHitTesting=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->SetSimulatePhysics(true);
		//GrabbedActorRight->SetActorEnableCollision(true);
		
		GrabbedActorRight->K2_DetachFromActor(EDetachmentRule::KeepWorld,EDetachmentRule::KeepWorld,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		//GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Right Tablet"))
	}
	// 오른손에 Fridge Door를 쥐고 있었다면
	else if(isGrabbingFridgeDoorRight)
	{
		isGrabbingFridgeDoorRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);
	}
	// 오른손에 Opener를 쥐고 있었다면
	else if(isGrabbingOpenerRight)
	{
		isGrabbingOpenerRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		float Angle;
		FVector Axis;
		DeltaRotation.ToAxisAndAngle(Axis, Angle);
		float dt = GetWorld()->DeltaTimeSeconds;
		FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Right Opener"))
	}
	// 오른손에 Coaster를 쥐고 있었다면
	else if(isGrabbingCoasterRight)
	{
		isGrabbingCoasterRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		float Angle;
		FVector Axis;
		DeltaRotation.ToAxisAndAngle(Axis, Angle);
		float dt = GetWorld()->DeltaTimeSeconds;
		FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Right Coaster"))
	}
	// 오른손에 Cup을 쥐고 있었다면
	else if(isGrabbingCupRight)
	{
		isGrabbingCupRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepWorld,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		//GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);

		UE_LOG(LogTemp, Warning, TEXT("release Right Cup"))
	}
	// 오른손에 BarSpoon을 쥐고 있었다면
	else if(isGrabbingBarSpoonRight)
	{
		isGrabbingBarSpoonRight=false;
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepWorld,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		//GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());
		//float Angle;
		//FVector Axis;
		//DeltaRotation.ToAxisAndAngle(Axis, Angle);
		//float dt = GetWorld()->DeltaTimeSeconds;
		//FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		//GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectRight = nullptr;
		GrabbedActorRight=nullptr;
		RightHandMesh->SetVisibility(true);
	}
	// 쥐고 있는 대상이 설정 대상 이외의 것이라면
	else
	{
		// 잡지않은 상태로 전환
		IsGrabbedRight = false;		
	}
}

// 던질 정보를 업데이트하기위한 기능
void ABarPlayer::Grabbing()
{
	if (IsGrabbedLeft&&IsGrabbedRight == false)
	{
		return;
	}

		if(IsGrabbedRight)
		{
			// 던질방향 업데이트
			ThrowDirection = RightHand->GetComponentLocation() - PrevPosRight;
			// 회전방향 업데이트
			// 쿼터니온 공식
			DeltaRotation = RightHand->GetComponentQuat() * PrevRotRight.Inverse();

			// 이전위치 업데이트
			PrevPosRight = RightHand->GetComponentLocation();
			// 이전회전값 업데이트
			PrevRotRight = RightHand->GetComponentQuat();
		}
		if(IsGrabbedLeft)
		{
			// 던질방향 업데이트
			ThrowDirectionLeft = LeftHand->GetComponentLocation() - PrevPosLeft;
			// 회전방향 업데이트
			// 쿼터니온 공식
			DeltaRotationLeft = LeftHand->GetComponentQuat() * PrevRotLeft.Inverse();

			// 이전위치 업데이트
			PrevPosLeft = LeftHand->GetComponentLocation();
			// 이전회전값 업데이트
			PrevRotLeft = LeftHand->GetComponentQuat();
		}

}

void ABarPlayer::Fire()
{	
	if(widgetInteractionComp)
	{
		//UI에 이벤트를 전달하고 싶다.
		widgetInteractionComp->PressPointerKey(FKey(FName("LeftMouseButton")));
			
	}
	
	// 왼손 혹은 오른손에 Tongs를 쥐고 있다면
	if(isGrabbingTongsRight)
	{
		// 중심점
		FVector Center = huchuTong->tongRight->GetSocketLocation(FName("TongAttach"));
		// 충돌체크(구충돌)
		// 충돌한 물체를 기억할 배열
		TArray<FOverlapResult> HitObj;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		params.AddIgnoredActor(huchuTong);
		//DrawDebugSphere(GetWorld(), Center, TongsGrabRange, 30, FColor::Red, false, 2.0, 0, 0.1);
		bool bHit = GetWorld()->OverlapMultiByChannel(HitObj, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TongsGrabRange), params);
		if (bHit == false)
		{
			return;
		}
		// 가장 가까운 물체를 잡도록 하자
		// 가장 가까운 물체 인덱스
		int32 Closest = 0;
		for (int i = 0; i < HitObj.Num(); ++i)
		{
			// 1. 물리기능이 활성화 되어 있는 대상만 판단
			if (HitObj[i].GetComponent()->IsSimulatingPhysics() == false)
			{
				continue;
			}
			// 잡기에 성공했다
			isGrabbingWithTongsRight = true;
			// 2.. 현재 손과 가장 가까운 대상과 이번에 검출할 대상과 더 가까운 대상이 있다면		
			// 필요속성 : 현재 가장 가까운 대상과 손과의 거리
			float ClosestDist = FVector::Dist(HitObj[Closest].GetActor()->GetActorLocation(), Center);
			// 필요속성 : 이번에 검출할 대상과 손과의 거리
			float NextDist = FVector::Dist(HitObj[i].GetActor()->GetActorLocation(), Center);

			// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
			if (NextDist < ClosestDist)
			{

				// 가장 가까운 대상으로 변경하기
				Closest = i;
			}
		}		
		// ice cube 잡기에 성공했다면
		if (isGrabbingWithTongsRight&&HitObj[Closest].GetComponent()->IsSimulatingPhysics() == true)
		{
			GrabbedActorWithTongsRight=HitObj[Closest].GetComponent()->GetAttachmentRootActor();
			GrabbedObjectWithTongsRight = HitObj[Closest].GetComponent();
			iceCube=Cast<AIceCube>(GrabbedActorWithTongsRight);
			if(GrabbedActorWithTongsRight==iceCube&&iceCube!=nullptr)
			{
				GrabbedObjectWithTongsRight->SetSimulatePhysics(false);
				GrabbedObjectWithTongsRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);				
				GrabbedObjectWithTongsRight->AttachToComponent(huchuTong->tongRight,FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("TongGrabSizeSocket"));
			}
		}		
			IsTongsMovementFinished=false;
			UE_LOG(LogTemp, Warning, TEXT("Huchu Fire"))
			FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;
			FLatentActionInfo LatentInfoL;
			LatentInfoL.CallbackTarget = this;
			LatentInfo.ExecutionFunction = FName(TEXT("TongsMovementExec"));
			LatentInfo.Linkage = 0;
			LatentInfo.UUID = 0; 
			auto tongCompRef = huchuTong->tongRight;
			auto tongCompRefL=huchuTong->tongLeft;
			auto tongLoc =  huchuTong->tongRight->GetSocketLocation(FName("TongGrabSizeSocket"));
			auto tongRightVector = huchuTong->GetActorForwardVector();
			FCollisionQueryParams params1;
			params1.AddIgnoredActor(huchuTong);  
			params1.AddIgnoredActor(this);				
			FHitResult leftTrace;
			FHitResult rightTrace;
			// Tongs 양쪽에서 LineTrace롤 통해 Grab할 대상의 크기를 측정한다.
			bool bHitR = GetWorld()->LineTraceSingleByChannel(rightTrace,tongLoc+tongRightVector*25.0f, tongLoc+tongRightVector*-25.0f, ECC_Visibility,params1);
			bool bHitL = GetWorld()->LineTraceSingleByChannel(leftTrace,tongLoc+tongRightVector*-25.0f, tongLoc+tongRightVector*25.0f, ECC_Visibility,params1);
			//DrawDebugLine(GetWorld(), tongLoc+tongRightVector*25.0f, tongLoc+tongRightVector*-25.0f, FColor::Red, false, 2.0f, 0, 0.5);
			// LineTrace가 양쪽 모두 적중했다면
			if(bHitL&&bHitR)
			{
				isTongsTickEnabled = false;
				// Left Impact Point와 Right Impact Point 사이의 간격을 도출한다
				grabbingObjectSize = FVector::Dist(leftTrace.ImpactPoint, rightTrace.ImpactPoint);
				// grabbingObjectSize에 따라서 Tongs가 다물어질 정도를 결정한다.
				UKismetSystemLibrary::MoveComponentTo(tongCompRef, tongCompRef->GetRelativeLocation(), tongCompRef->GetRelativeRotation()+FRotator(grabbingObjectSize/30, 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfo);
				UKismetSystemLibrary::MoveComponentTo(tongCompRefL, tongCompRefL->GetRelativeLocation(), tongCompRefL->GetRelativeRotation()+FRotator(-(grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfoL);
				UE_LOG(LogTemp, Warning, TEXT("grabbingObjectSize : %f"), grabbingObjectSize)
			}
			// LineTrace가 적중하지 않았다면 -> 허공이라면
			else
			{
				// Oculus Trigger Input Value에 따른 Tongs Rotation 제어 Tick 활성화
				isTongsTickEnabled=true;

			}

	}
		// 오른손에 Tongs를 쥐고 있지 않다면
		else
		{
			return;
		}
	
}

void ABarPlayer::FireLeft()
{
	// 왼손에 Tongs를 쥐고 있다면
	if(isGrabbingTongsLeft)
	{
		// 중심점
		FVector Center = huchuTongL->tongRight->GetSocketLocation(FName("TongAttach"));
		// 충돌체크(구충돌)
		// 충돌한 물체를 기억할 배열
		TArray<FOverlapResult> HitObj;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		params.AddIgnoredActor(huchuTongL);
		//DrawDebugSphere(GetWorld(), Center, TongsGrabRange, 30, FColor::Red, false, 2.0, 0, 0.1);
		bool bHit = GetWorld()->OverlapMultiByChannel(HitObj, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TongsGrabRange), params);
		if (bHit == false)
		{
			return;
		}
		// 가장 가까운 물체를 잡도록 하자
		// 가장 가까운 물체 인덱스
		int32 Closest = 0;
		for (int i = 0; i < HitObj.Num(); ++i)
		{
			// 1. 물리기능이 활성화 되어 있는 대상만 판단
			if (HitObj[i].GetComponent()->IsSimulatingPhysics() == false)
			{
				continue;
			}
			// 잡기에 성공했다
			isGrabbingWithTongsLeft = true;
			// 2.. 현재 손과 가장 가까운 대상과 이번에 검출할 대상과 더 가까운 대상이 있다면		
			// 필요속성 : 현재 가장 가까운 대상과 손과의 거리
			float ClosestDist = FVector::Dist(HitObj[Closest].GetActor()->GetActorLocation(), Center);
			// 필요속성 : 이번에 검출할 대상과 손과의 거리
			float NextDist = FVector::Dist(HitObj[i].GetActor()->GetActorLocation(), Center);

			// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
			if (NextDist < ClosestDist)
			{

				// 가장 가까운 대상으로 변경하기
				Closest = i;
			}
		}

		// ice cube 잡기에 성공했다면
		if (isGrabbingWithTongsLeft&&HitObj[Closest].GetComponent()->IsSimulatingPhysics() == true)
		{
			GrabbedActorWithTongsLeft=HitObj[Closest].GetComponent()->GetAttachmentRootActor();
			GrabbedObjectWithTongsLeft = HitObj[Closest].GetComponent();
			iceCubeL=Cast<AIceCube>(GrabbedActorWithTongsLeft);
			if(GrabbedActorWithTongsLeft==iceCubeL&&iceCubeL!=nullptr)
			{
				GrabbedObjectWithTongsLeft->SetSimulatePhysics(false);
				GrabbedObjectWithTongsLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				GrabbedObjectWithTongsLeft->AttachToComponent(huchuTongL->tongRight,FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("TongGrabSizeSocket"));
			}
		}	
			UE_LOG(LogTemp, Warning, TEXT("Huchu Fire Left"))
			FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;
			FLatentActionInfo LatentInfoL;
			LatentInfoL.CallbackTarget = this;
			auto tongCompRef = huchuTongL->tongRight;
			auto tongCompRefL=huchuTongL->tongLeft;
			auto tongLoc =  huchuTongL->tongRight->GetSocketLocation(FName("TongGrabSizeSocket"));
			auto tongRightVector = huchuTongL->GetActorForwardVector();
			FCollisionQueryParams params1;
			params1.AddIgnoredActor(huchuTongL);  
			params1.AddIgnoredActor(this);				
			FHitResult leftTrace;
			FHitResult rightTrace;
			// Tongs 양쪽에서 LineTrace롤 통해 Grab할 대상의 크기를 측정한다.
			bool bHitR = GetWorld()->LineTraceSingleByChannel(rightTrace,tongLoc+tongRightVector*25.0f, tongLoc+tongRightVector*-25.0f, ECC_Visibility,params1);
			bool bHitL = GetWorld()->LineTraceSingleByChannel(leftTrace,tongLoc+tongRightVector*-25.0f, tongLoc+tongRightVector*25.0f, ECC_Visibility,params1);
			//DrawDebugLine(GetWorld(), tongLoc+tongRightVector*25.0f, tongLoc+tongRightVector*-25.0f, FColor::Red, false, 2.0f, 0, 0.5);
			// LineTrace가 양쪽 모두 적중했다면
			if(bHitL&&bHitR)
			{
				isTongsTickEnabledL = false;
				// Left Impact Point와 Right Impact Point 사이의 간격을 도출한다
				grabbingObjectSizeL = FVector::Dist(leftTrace.ImpactPoint, rightTrace.ImpactPoint);
				// grabbingObjectSize에 따라서 Tongs가 다물어질 정도를 결정한다.
				UKismetSystemLibrary::MoveComponentTo(tongCompRef, tongCompRef->GetRelativeLocation(), tongCompRef->GetRelativeRotation()+FRotator(grabbingObjectSize/30, 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfo);
				UKismetSystemLibrary::MoveComponentTo(tongCompRefL, tongCompRefL->GetRelativeLocation(), tongCompRefL->GetRelativeRotation()+FRotator(-(grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfoL);
				UE_LOG(LogTemp, Warning, TEXT("grabbingObjectSizeLeft : %f"), grabbingObjectSizeL)
			}
			// LineTrace가 적중하지 않았다면 -> 허공이라면
			else
			{
				// Oculus Trigger Input Value에 따른 Tongs Rotation 제어 Tick 활성화
				isTongsTickEnabledL=true;

			}

	}
		// 오른손에 Tongs를 쥐고 있지 않다면
		else
		{
			return;
		}
}

void ABarPlayer::FireReleased()
{
	//클릭 버튼 떼어내기 위한 함수
	if (widgetInteractionComp)
	{
		widgetInteractionComp->ReleasePointerKey(FKey(FName("LeftMouseButton")));
	}
	if(isGrabbingTongsRight)
	{		
			// Tongs로 잡고 있는 대상이 있었다면
			if (isGrabbingWithTongsRight)
			{
				IsTongsReleaseMovementFinished=false;
				UE_LOG(LogTemp, Warning, TEXT("Huchu Fire Released"))
				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;
				FLatentActionInfo LatentInfoL;
				LatentInfoL.CallbackTarget = this;
				LatentInfo.ExecutionFunction = FName(TEXT("TongsReleaseMovementExec"));
				LatentInfo.Linkage = 0;
				LatentInfo.UUID = 0; 
				auto tongCompRef = huchuTong->tongRight;
				auto tongCompRefL=huchuTong->tongLeft;
				UKismetSystemLibrary::MoveComponentTo(tongCompRef, tongCompRef->GetRelativeLocation(), tongCompRef->GetRelativeRotation()+FRotator(-(grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfo);
				UKismetSystemLibrary::MoveComponentTo(tongCompRefL, tongCompRefL->GetRelativeLocation(), tongCompRefL->GetRelativeRotation()+FRotator((grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfoL);
				isTongsTickEnabled = true;
				grabbingObjectSize = 0;
				// 1. 잡지않은 상태로 전환
				isGrabbingWithTongsRight = false;
				// 2. 손에서 떼어내기
				GrabbedObjectWithTongsRight->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				// 3. 물리기능 활성화
				GrabbedObjectWithTongsRight->SetSimulatePhysics(true);
				// 4. 충돌기능 활성화
				GrabbedObjectWithTongsRight->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				GrabbedObjectWithTongsRight = nullptr;
			}
			// Tongs 로 잡고 있는 대상이 없었다면
			else
			{
				isGrabbingWithTongsRight = false;
				IsTongsReleaseMovementFinished=false;
				UE_LOG(LogTemp, Warning, TEXT("Huchu Fire Released"))
				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;
				FLatentActionInfo LatentInfoL;
				LatentInfoL.CallbackTarget = this;
				isTongsTickEnabled = true;
				grabbingObjectSize = 0;
			}	
		}
		else
		{
			return;
		}
	
}

void ABarPlayer::FireReleasedLeft()
{
		if(isGrabbingTongsLeft)
		{		
			// Tongs로 잡고 있는 대상이 있었다면
			if (isGrabbingWithTongsLeft)
			{
				UE_LOG(LogTemp, Warning, TEXT("Huchu Fire Released Left"))
				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;
				FLatentActionInfo LatentInfoL;
				LatentInfoL.CallbackTarget = this;
				auto tongCompRef = huchuTongL->tongRight;
				auto tongCompRefL=huchuTongL->tongLeft;
				UKismetSystemLibrary::MoveComponentTo(tongCompRef, tongCompRef->GetRelativeLocation(), tongCompRef->GetRelativeRotation()+FRotator(-(grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfo);
				UKismetSystemLibrary::MoveComponentTo(tongCompRefL, tongCompRefL->GetRelativeLocation(), tongCompRefL->GetRelativeRotation()+FRotator((grabbingObjectSize/30), 0, 0), false, false, 0.0, false, EMoveComponentAction::Move, LatentInfoL);
				isTongsTickEnabledL = true;
				grabbingObjectSizeL = 0;
				// 1. 잡지않은 상태로 전환
				isGrabbingWithTongsLeft = false;
				// 2. 손에서 떼어내기
				GrabbedObjectWithTongsLeft->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				// 3. 물리기능 활성화
				GrabbedObjectWithTongsLeft->SetSimulatePhysics(true);
				// 4. 충돌기능 활성화
				GrabbedObjectWithTongsLeft->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				GrabbedObjectWithTongsLeft = nullptr;
			}
			// Tongs 로 잡고 있는 대상이 없었다면
			else
			{
				isGrabbingWithTongsLeft = false;
				UE_LOG(LogTemp, Warning, TEXT("Huchu Fire Released Left"))
				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;
				FLatentActionInfo LatentInfoL;
				LatentInfoL.CallbackTarget = this;
				isTongsTickEnabledL = true;
				grabbingObjectSizeL = 0;
			}	
		}
		else
		{
			return;
		}
}

