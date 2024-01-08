// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	//Line trace colliide with box to show widget
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//Item popup widget 
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	//Enables item tracing when overlapped
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class USphereComponent AreaSphere;


public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
};
