// Fill out your copyright notice in the Description page of Project Settings.


#include "BarPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "HuchuTong.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"


// Sets default values
ABarPlayer::ABarPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

	RightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightAim"));
	RightAim->SetupAttachment(RootComponent);
	RightAim->SetTrackingMotionSource(FName("RightAim"));

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_QuinnXR_left.SKM_QuinnXR_left'"));
	if (TempMesh.Succeeded())
	{
		LeftHandMesh->SetSkeletalMesh(TempMesh.Object);
		LeftHandMesh->SetRelativeLocation(FVector(-2.9f, -3.5f, 4.5f));
		LeftHandMesh->SetRelativeRotation(FRotator(-25, -180, 90));
	}

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh2(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_QuinnXR_right.SKM_QuinnXR_right'"));
	if (TempMesh2.Succeeded())
	{
		RightHandMesh->SetSkeletalMesh(TempMesh2.Object);
		RightHandMesh->SetRelativeLocation(FVector(-2.9f, 3.5f, 4.5f));
		RightHandMesh->SetRelativeRotation(FRotator(25, 0, 90));
	}

	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	FPSCamera->SetupAttachment(RootComponent);
	FPSCamera->SetRelativeLocation(FVector(0, 0, 30));
	FPSCamera->bUsePawnControlRotation = false;

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

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		RightAim->SetRelativeLocation(FVector(20, 20, 0));
		RightHand->SetRelativeLocation(FVector(20, 20, 0));
		FPSCamera->bUsePawnControlRotation = true;

		FPSCamera->AddRelativeLocation(FVector(0, 0, 88));


	}
	else
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		FPSCamera->bUsePawnControlRotation = false;
	}
	
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
	
	// 잡은 대상 위치값 실시간 업데이트
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
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
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


void ABarPlayer::TryGrabLeft()
{
	// 중심점
	FVector Center = LeftHand->GetComponentLocation();
	// 충돌체크(구충돌)
	// 충돌한 물체를 기억할 배열
	TArray<FOverlapResult> HitObj;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	params.AddIgnoredComponent(LeftHand);
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
		float NextDist = FVector::Dist(HitObj[0].GetActor()->GetActorLocation(), Center);

		// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
		if (NextDist < ClosestDist)
		{

			// 가장 가까운 대상으로 변경하기
			Closest = i;
		}
	}

	// 잡기에 성공했다면
	if (IsGrabbedLeft)
	{
		// 물체 물리기능 비활성화
		GrabbedActorLeft=HitObj[Closest].GetActor();
		GrabbedObjectLeft = HitObj[Closest].GetComponent();
		GrabbedObjectLeft->SetSimulatePhysics(false);
		GrabbedObjectLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Left Tong Casting
		huchuTongL=Cast<AHuchuTong>(GrabbedActorLeft);
		// 잡은 대상이 Tongs라면
		if(GrabbedActorLeft==huchuTongL&&huchuTongL!=nullptr)
		{
			isGrabbingTongsLeft=true;
			GrabbedObjectWithTongsLeft = nullptr;
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("TongsSocket"),EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,EAttachmentRule::KeepRelative,true);
			LeftHandMesh->SetVisibility(false);
			GrabbedActorLeft->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab huchu on Left"))
		}
		else
		{
			GrabbedObjectLeft->K2_AttachToComponent(LeftHandMesh, TEXT("CompGrabSocket"),EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);

		}
		
	}

}

void ABarPlayer::TryGrabRight()
{
	// 중심점
	FVector Center = RightHand->GetComponentLocation();
	// 충돌체크(구충돌)
	// 충돌한 물체를 기억할 배열
	TArray<FOverlapResult> HitObj;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	params.AddIgnoredComponent(RightHand);
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
		float NextDist = FVector::Dist(HitObj[0].GetActor()->GetActorLocation(), Center);

		// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
		if (NextDist < ClosestDist)
		{

			// 가장 가까운 대상으로 변경하기
			Closest = i;
		}
	}

	// 잡기에 성공했다면
	if (IsGrabbedRight)
	{
		// 물체 물리기능 비활성화
		GrabbedActorRight=HitObj[Closest].GetActor();
		GrabbedObjectRight = HitObj[Closest].GetComponent();
		GrabbedObjectRight->SetSimulatePhysics(false);
		// 손에 붙여주자
		// Tong Casting
		huchuTong=Cast<AHuchuTong>(GrabbedActorRight);
		// 잡은 대상이 Tongs라면
		if(GrabbedActorRight==huchuTong&&huchuTong!=nullptr)
		{
			isGrabbingTongsRight=true;
			GrabbedObjectWithTongsRight = nullptr;
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("TongsSocket"),EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,EAttachmentRule::KeepRelative,true);
			RightHandMesh->SetVisibility(false);
			GrabbedActorRight->SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("grab huchu on Right"))
		}
		else
		{
			GrabbedObjectRight->K2_AttachToComponent(RightHandMesh, TEXT("CompGrabSocket"),EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,true);

		}

	}

}
void ABarPlayer::UnTryGrabLeft()
{
	if (IsGrabbedLeft == false)
	{
		return;
	}
	// 왼손에 Tongs를 잡고 있었다면
	if(isGrabbingTongsLeft)
	{		
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
		IsGrabbedLeft = false;
		GrabbedObjectLeft->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectLeft->SetSimulatePhysics(true);
		GrabbedActorLeft->SetActorEnableCollision(true);
		GrabbedObjectLeft = nullptr;
		LeftHandMesh->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("release Left huchu"))
	}
	else
	{
		// 1. 잡지않은 상태로 전환
		IsGrabbedLeft = false;
		// 2. 손에서 떼어내기
		GrabbedObjectLeft->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// 3. 물리기능 활성화
		GrabbedObjectLeft->SetSimulatePhysics(true);
		// 4. 충돌기능 활성화
		GrabbedObjectLeft->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 던지기
		GrabbedObjectLeft->AddForce(ThrowDirectionLeft * ThrowPower * GrabbedObjectLeft->GetMass());

		// 회전 시키기
		// 각속도 = (1 / dt) * dTheta(특정 축 기준 변위 각도 Axis, angle)
		float Angle;
		FVector Axis;
		DeltaRotationLeft.ToAxisAndAngle(Axis, Angle);
		float dt = GetWorld()->DeltaTimeSeconds;
		FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		GrabbedObjectLeft->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);
		GrabbedObjectLeft = nullptr;
	}
}

void ABarPlayer::UnTryGrabRight()
{	
	if (IsGrabbedRight == false)
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
		IsGrabbedRight = false;
		GrabbedObjectRight->K2_DetachFromComponent(EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative,EDetachmentRule::KeepRelative);
		GrabbedObjectRight->SetSimulatePhysics(true);
		GrabbedActorRight->SetActorEnableCollision(true);
		GrabbedObjectRight = nullptr;
		RightHandMesh->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("release Right huchu"))
	}
	else
	{
		// 1. 잡지않은 상태로 전환
		IsGrabbedRight = false;
		// 2. 손에서 떼어내기
		GrabbedObjectRight->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// 3. 물리기능 활성화
		GrabbedObjectRight->SetSimulatePhysics(true);
		// 4. 충돌기능 활성화
		GrabbedObjectRight->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 던지기
		GrabbedObjectRight->AddForce(ThrowDirection * ThrowPower * GrabbedObjectRight->GetMass());

		// 회전 시키기
		// 각속도 = (1 / dt) * dTheta(특정 축 기준 변위 각도 Axis, angle)
		float Angle;
		FVector Axis;
		DeltaRotation.ToAxisAndAngle(Axis, Angle);
		float dt = GetWorld()->DeltaTimeSeconds;
		FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
		GrabbedObjectRight->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);

		GrabbedObjectRight = nullptr;
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
		DrawDebugSphere(GetWorld(), Center, TongsGrabRange, 30, FColor::Red, false, 2.0, 0, 0.1);
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
			float NextDist = FVector::Dist(HitObj[0].GetActor()->GetActorLocation(), Center);

			// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
			if (NextDist < ClosestDist)
			{

				// 가장 가까운 대상으로 변경하기
				Closest = i;
			}
		}

		// 잡기에 성공했다면
		if (isGrabbingWithTongsRight)
		{
			// 물체 물리기능 비활성화
			GrabbedObjectWithTongsRight = HitObj[Closest].GetComponent();
			GrabbedObjectWithTongsRight->SetSimulatePhysics(false);
			GrabbedObjectWithTongsRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);				
			GrabbedObjectWithTongsRight->AttachToComponent(huchuTong->tongRight,FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("TongGrabSizeSocket"));
								
		}		
			FVector tongAttachLoc = huchuTong->tongRight->GetSocketLocation(FName("TongAttach"));
			FRotator tongAttachRot = huchuTong->tongRight->GetSocketRotation(FName("TongAttach"));
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
			DrawDebugLine(GetWorld(), tongLoc+tongRightVector*25.0f, tongLoc+tongRightVector*-25.0f, FColor::Red, false, 2.0f, 0, 0.5);
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
		DrawDebugSphere(GetWorld(), Center, TongsGrabRange, 30, FColor::Red, false, 2.0, 0, 0.1);
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
			float NextDist = FVector::Dist(HitObj[0].GetActor()->GetActorLocation(), Center);

			// 3. 만약 이번 대상이 현재 대상보다 가깝다면,
			if (NextDist < ClosestDist)
			{

				// 가장 가까운 대상으로 변경하기
				Closest = i;
			}
		}

		// 잡기에 성공했다면
		if (isGrabbingWithTongsLeft)
		{
			// 물체 물리기능 비활성화
			GrabbedObjectWithTongsLeft = HitObj[Closest].GetComponent();
			GrabbedObjectWithTongsLeft->SetSimulatePhysics(false);
			GrabbedObjectWithTongsLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);				
			GrabbedObjectWithTongsLeft->AttachToComponent(huchuTongL->tongRight,FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("TongGrabSizeSocket"));
								
		}		
			FVector tongAttachLoc = huchuTongL->tongRight->GetSocketLocation(FName("TongAttach"));
			FRotator tongAttachRot = huchuTongL->tongRight->GetSocketRotation(FName("TongAttach"));
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
			DrawDebugLine(GetWorld(), tongLoc+tongRightVector*25.0f, tongLoc+tongRightVector*-25.0f, FColor::Red, false, 2.0f, 0, 0.5);
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

