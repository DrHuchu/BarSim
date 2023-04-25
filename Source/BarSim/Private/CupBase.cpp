// Fill out your copyright notice in the Description page of Project Settings.


#include "CupBase.h"

#include "DropBase.h"
#include "IceCube.h"
#include "VorbisAudioInfo.h"
#include "Components/BoxComponent.h"

// Sets default values
ACupBase::ACupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	cupComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cup"));
	SetRootComponent(cupComp);

	liquorComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Liquor"));
	liquorComp->SetupAttachment(cupComp);
	
	measureComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Measure"));
	measureComp->SetupAttachment(cupComp);

	igCheckerComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IGChecker"));
	igCheckerComp->SetupAttachment(cupComp);
}

// Called when the game starts or when spawned
void ACupBase::BeginPlay()
{
	Super::BeginPlay();

	cupSize = cupSizeOrigin;

	liquorComp->SetVisibility(false);
	measureComp->OnComponentBeginOverlap.AddDynamic(this, &ACupBase::AddLiquor);
	igCheckerComp->OnComponentBeginOverlap.AddDynamic(this, &ACupBase::AddIce);
	igCheckerComp->OnComponentEndOverlap.AddDynamic(this, &ACupBase::ExtractIce);
}

// Called every frame
void ACupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACupBase::AddLiquor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	drop = Cast<ADropBase>(OtherActor);

	if(drop)
	{
		overlappedNum ++;
		//들어온 방울 액터가 가진 이름을 OrderArrary에, 유량을 ContentsArray에 저장한다.
		//OrderArray 배열의 마지막 원소와 현재 닿은 물방울의 이름이 같다면
		if(!OrderArray.IsEmpty())
		{
			if(OrderArray[OrderArray.Num()-1] == drop->name)
			{
				//들어온 방울 액터의 이름이 마지막 배열 원소의 이름과 같다면 그 배열에 ContentsArray에 더한다.
				ContentsArray[ContentsArray.Num()-1] += drop->dropMass;
			}
			else
			{
				//들어온 방울 액터의 이름이 마지막 배열 원소의 이름과 다르다면 다음 원소로 orderArray와 ContentsArray를 넣는다.
				OrderArray.Emplace(drop->name);
				ContentsArray.Add(drop->dropMass);
			}
		}
		else
			//배열이 비어있을 경우 값 하나 일단 넣기
		{
			OrderArray.Emplace(drop->name);
			ContentsArray.Add(drop->dropMass);
		}
		contents = contents + drop->dropMass;
		float insideContents = FMath::Clamp(contents, 0, cupSize);
		liquorComp->SetVisibility(true);
		liquorComp->SetRelativeScale3D(FVector(1,1,insideContents / cupSize));
		UE_LOG(LogTemp, Warning, TEXT("%f"), insideContents);
		drop->Destroy();
	}
}

void ACupBase::AddIce(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ice = Cast<AIceCube>(OtherActor);
	//igchecker에 얼음이 오버랩되었을 때
	if(ice)
	{
		iceCount += 1;
		//얼음 갯수 하나당 2온스씩 내부 용량 줄이기
		cupSize = cupSizeOrigin - iceCount * 2;
		float insideContents = FMath::Clamp(contents, 0, cupSize);
		liquorComp->SetRelativeScale3D(FVector(1,1,insideContents / cupSize));
	}
}

void ACupBase::ExtractIce(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ice = Cast<AIceCube>(OtherActor);
	//igchecker에 얼음이 오버랩해제되었을 때
	if(ice)
	{
		iceCount -= 1;
		//얼음 갯수 하나당 2온스씩 내부 용량 줄이기
		cupSize = cupSizeOrigin - iceCount * 2;
		float insideContents = FMath::Clamp(contents, 0, cupSize);
		liquorComp->SetRelativeScale3D(FVector(1,1,insideContents / cupSize));
	}
}

