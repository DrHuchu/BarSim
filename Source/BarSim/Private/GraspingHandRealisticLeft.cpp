// Fill out your copyright notice in the Description page of Project Settings.


#include "GraspingHandRealisticLeft.h"

#include "ActorInfoWidget.h"
#include "BarSpoon.h"
#include "BottleBase.h"
#include "Coaster.h"
#include "CupBase.h"
#include "HalfSlicedLime.h"
#include "HuchuTong.h"
#include "IceCube.h"
#include "MixingGlass.h"
#include "Shaker.h"
#include "ShakerLid.h"
#include "ShakerStrainer.h"
#include "SlicedLime.h"
#include "Strainer.h"
#include "Tablet.h"
#include "Components/BoxComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"

AGraspingHandRealisticLeft::AGraspingHandRealisticLeft(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	actorInfoWidgetComp=CreateDefaultSubobject<UWidgetComponent>(TEXT("actorInfoWidgetComp"));
	actorInfoWidgetComp->SetupAttachment(RootComponent);

	boxComp=CreateDefaultSubobject<UBoxComponent>(TEXT("boxComp"));
	boxComp->SetupAttachment(RootComponent);
	boxComp->SetGenerateOverlapEvents(true);
}

void AGraspingHandRealisticLeft::BeginPlay()
{
	Super::BeginPlay();

	actorInfoWidget=Cast<UActorInfoWidget>(actorInfoWidgetComp->GetWidget());
	
	skeletalHand=GetSkeletalMeshComponent();	
	skeletalHand->SetGenerateOverlapEvents(true);
	skeletalHand->OnComponentBeginOverlap.AddDynamic(this, &AGraspingHandRealisticLeft::OnOverlap);
	actorInfoWidgetComp->SetVisibility(false);
}

void AGraspingHandRealisticLeft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void AGraspingHandRealisticLeft::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TArray<FHitResult> hits;
	if(SweepResult.GetNumOverlapHits(hits)>1)
	{
		FVector Center = skeletalHand->GetComponentLocation();
		int32 Closest = 0;
		for(int i=0; i<hits.Num(); ++i)
		{
			float ClosestDist = FVector::Dist(hits[Closest].GetActor()->GetActorLocation(), Center);
			float NextDist = FVector::Dist(hits[i].GetActor()->GetActorLocation(), Center);

			if (NextDist < ClosestDist)
			{
				Closest = i;
			}
		}

		OtherActor=hits[Closest].GetActor();
	}
	// Overlapped Actor Cast
	huchuTong=Cast<AHuchuTong>(OtherActor);
	bottle = Cast<ABottleBase>(OtherActor);
	tablet = Cast<ATablet>(OtherActor);
	coaster=Cast<ACoaster>(OtherActor);
	cup=Cast<ACupBase>(OtherActor);
	barSpoon=Cast<ABarSpoon>(OtherActor);
	strainer=Cast<AStrainer>(OtherActor);
	shakerStrainer=Cast<AShakerStrainer>(OtherActor);
	shakerLid=Cast<AShakerLid>(OtherActor);
	shaker=Cast<AShaker>(OtherActor);
	mixingGlass=Cast<AMixingGlass>(OtherActor);
	iceCube=Cast<AIceCube>(OtherActor);
	slicedLime=Cast<ASlicedLime>(OtherActor);
	halfSlicedLime=Cast<AHalfSlicedLime>(OtherActor);

	if(huchuTong)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("집게"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);
	}
	else if(barSpoon)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("바 스푼"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(tablet)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("태블릿"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(coaster)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("코스터"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(strainer)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("스트레이너"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(shakerStrainer)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("쉐이커 스트레이너"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(shakerLid)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("쉐이커 뚜껑"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(shaker)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("쉐이커"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(mixingGlass)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("믹싱 글래스"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(iceCube)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("얼음"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(halfSlicedLime)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("라임"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}
	else if(slicedLime)
	{
		GetWorldTimerManager().ClearTimer(widgetDestroyHandle);
		actorInfoWidget->ActorInfo->SetText(FText::FromString(FString::Printf(TEXT("라임"))));
		actorInfoWidgetComp->SetVisibility(true);
		GetWorldTimerManager().SetTimer(widgetDestroyHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			actorInfoWidgetComp->SetVisibility(false);
		}), widgetVisibilityTime, false);	}	
	
	
	
}


