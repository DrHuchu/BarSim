// Fill out your copyright notice in the Description page of Project Settings.


#include "Chair.h"
#include "Coaster.h"
#include "CoctailScoreWidget.h"
#include "CupBase.h"
#include "CustomerCharacter.h"
#include "CustomerFSM.h"
#include "PlayerCharacter.h"
#include "SpawnManager.h"
#include "Components/BoxComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChair::AChair()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("boxComp"));
	SetRootComponent(boxComp);

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("meshComp"));
	meshComp->SetupAttachment(boxComp);

	sitComp = CreateDefaultSubobject<USceneComponent>(TEXT("sitComp"));
	sitComp->SetupAttachment(meshComp);
	
	coctailBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("coctailBoxComp"));
	coctailBoxComp->SetupAttachment(boxComp);

	playerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("playerBoxComp"));
	playerBoxComp->SetupAttachment(boxComp);

	coctailWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CoctailWidget"));
	coctailWidget->SetupAttachment(coctailBoxComp);
}

// Called when the game starts or when spawned
void AChair::BeginPlay()
{
	Super::BeginPlay();

	// 손님 오버랩 함수 바인드
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &AChair::OnCustomerOverlap);

	// 캌테일존 오버랩 함수 바인드
	coctailBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AChair::OnCupOverlap);
	coctailBoxComp->OnComponentEndOverlap.AddDynamic(this, &AChair::EndCupOverlap);

	// 플레이어 오버랩 함수 바인드
	playerBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AChair::OnPlayerOverlap);
	playerBoxComp->OnComponentEndOverlap.AddDynamic(this, &AChair::EndPlayerOverlap);

	score_UI = Cast<UCoctailScoreWidget>(coctailWidget->GetUserWidgetObject());

	spawnManager = Cast<ASpawnManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASpawnManager::StaticClass()));
}

// Called every frame
void AChair::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChair::OnCustomerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto customer = Cast<ACustomerCharacter>(OtherActor);

	if(customer != nullptr)
	{
		// 손님이 오버랩 되면 손님의 요소 스폰매니저로 보내기
		// 주문한 칵테일 저장
		auto orderTemp = customer->customerFSM->orderIdx;

		// 손님의 위치 순서 저장
		customerIdx = customer->customerFSM->idx;

		spawnManager->GetCustomerIdx(orderTemp, customerIdx);

		bCheckCustomer = true;
	}
}

void AChair::EndCustomerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	auto customer = Cast<ACustomerCharacter>(OtherActor);

	if(customer != nullptr)
	{
		bCheckCustomer = false;
	}
}

void AChair::OnCupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto tempCoaster = Cast<ACoaster>(OtherActor);
	auto tempCoctail = Cast<ACupBase>(OtherActor);

	// 칵테일잔이 있고 코스터가 있고 한번만 오버랩 되었다면
	if(tempCoctail != nullptr && bOnceOverlap != true && bCheckCoaster != false && bCheckCustomer != false)
	{
		coctail = tempCoctail;
		
		bCheckCoctail = true;

		// 컵 정보를 보내준다
		spawnManager->GetCup(coctail->NameArray, coctail->ContentsArray, coctail->bStirred, coctail->bStirredLater, coctail->bShaked, customerIdx);

		bOnceOverlap = true;
	}
	else if(tempCoaster != nullptr)
	{
		coaster = tempCoaster;
		
		bCheckCoaster = true;
	}
}

void AChair::EndCupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	auto tempCoaster = Cast<ACoaster>(OtherActor);
	auto tempCoctail = Cast<ACupBase>(OtherActor);

	if(tempCoctail != nullptr)
	{
		coctail = nullptr;
		
		bCheckCoctail = false;
	}
	if(tempCoaster != nullptr)
	{
		coaster = nullptr;
		
		bCheckCoaster = false;	
	}
}

void AChair::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto target = Cast<APlayerCharacter>(OtherActor);

	if(target != nullptr)
	{
		bCheckPlayer = true;
	}
}

void AChair::EndPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	auto target = Cast<APlayerCharacter>(OtherActor);

	if(target != nullptr)
	{
		bCheckPlayer = false;
	}
}

// 점수 보이는 함수
void AChair::ViewScore(int32 score)
{
	totalScore = score;
	
	score_UI->SetVisibility(ESlateVisibility::Visible);
	
	score_UI->text_Score->SetText(FText::AsNumber(score));
}

// 점수 가리기 함수
void AChair::HideScore()
{
	score_UI->SetVisibility(ESlateVisibility::Hidden);
}

// 주문대로 나왔을때 변수 변경 함수
void AChair::SameOrder()
{
	bSameOrder = true;
}

// 주문대로 안나왔을때 변수 변경 함수
void AChair::UnSameOrder()
{
	bUnSameOrder = true;
}

// 컵 손님 앞으로 이동시키는 함수
void AChair::MoveCup()
{
	FVector targetLoc = coctailBoxComp->GetComponentLocation() + GetActorForwardVector() * -10 + GetActorRightVector() * 5;

	coctail->SetActorLocation(FVector(targetLoc.X, targetLoc.Y, coctail->GetActorLocation().Z));

	coaster->SetActorLocation(FVector(targetLoc.X, targetLoc.Y, coaster->GetActorLocation().Z));
}