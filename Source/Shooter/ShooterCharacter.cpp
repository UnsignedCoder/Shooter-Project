// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values for the ShooterCharacter class
AShooterCharacter::AShooterCharacter() :
	// Camera FOV values
	bAiming(false), CameraDefaultFOV(0.0f), CameraZoomedFOV(35.0f), CameraCurrentFOV(0.0f), ZoomInterpSpeed(30.0f),
	// Aim Sensitivity Values
	HipFireSensitivity(1.0f), ADSSensitivity(0.45f), CurrentAimSensitivity(HipFireSensitivity),

	//Crosshair Spread Factors
	CrosshairSpreadMultiplier(0.0f), CrosshairInAirFactor(0.0f), CrosshairAimingFactor(0.0f), CrosshairShootingFactor(0.0f),

	//Bullet timer variables
	ShootTimeDuration(0.05f), bFiringBullet(false),

	//Automatic fire variables
	bShouldFire(true), AutomaticFireRate(0.1f), bFireButtonPressed(false),

	//Item Trace Variables
	bShouldTraceForItems(false)

{
	// Set this character to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the camera if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.0f; // The camera follows at this distance to the character
	CameraBoom->bUsePawnControlRotation = true; // Rotates the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f); // Camera offset from the character

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate character to controller rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input at the rotation rate defined below
	GetCharacterMovement()->RotationRate = FRotator(0.f, 560.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay() {
	Super::BeginPlay();

	// Initialize default camera field of view
	if (FollowCamera) {
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Check if the PlayerInputComponent is valid
	check(PlayerInputComponent);

	// Bind input actions and axis to corresponding functions
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::TurnRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUpRate);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);

	// Adjust sensitivity based on aiming state
	SensitivitySetting();

	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);

	// Trace for items if overlapped
	TraceForItems();
}

// Function to handle when the aiming button is pressed
void AShooterCharacter::AimingButtonPressed() {
	bAiming = true;
}

// Function to handle when the aiming button is released
void AShooterCharacter::AimingButtonReleased() {
	bAiming = false;
}

void AShooterCharacter::FireButtonPressed() {
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased() {
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer() {
	if (bShouldFire) {
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTImer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset() {
	bShouldFire = true;

	if (bFireButtonPressed) {
		StartFireTimer();
	}
}

// Function to adjust sensitivity based on the aiming state
void AShooterCharacter::SensitivitySetting() {
	if (bAiming) {
		CurrentAimSensitivity = ADSSensitivity; // Aiming Down Sight (ADS) sensitivity
	}
	else {
		CurrentAimSensitivity = HipFireSensitivity; // Hip-fire sensitivity
	}
}

// Function to interpolate the aim field of view and is called every frame
void AShooterCharacter::CameraInterpZoom(float DeltaTime) {
	if (bAiming) {
		// Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else {
		// Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

// Function to calculate crosshair spread multiplier
void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime) {
	FVector2D WalkSpeedRange{ 0.0f, 600.0f };
	FVector2D VelocityMultiplierRange{ 0.0f, 1.0f };
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;

	//Crosshair Velocity Factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//Crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) {
		//Spread the crosshair slowly in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else {
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
	}

	//Calculate aiming crosshair spread factor
	if (bAiming) {
		CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, -0.5f, DeltaTime, 30.0f);
	}
	else {
		CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.0f, DeltaTime, 30.0f);
	}

	//True 0.05 after firing bullet
	if (bFiringBullet) {
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.0f);
	}
	else {
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 60.0f);

		CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimingFactor + CrosshairShootingFactor;
	}
}

void AShooterCharacter::StartCrosshairBulletFire() {
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire() {
	bFiringBullet = false;
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) {
	//Get VIewport SIze
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get Screen Space Location of Cross-hairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	CrosshairLocation.Y -= 50.0f; // Adjust the vertical position of the crosshair
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Convert screen space crosshair location to world space
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation,
		CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld) {
		//Line trace from crosshairs world location
		const FVector Start = CrosshairWorldPosition;
		const FVector End = Start + CrosshairWorldDirection * 50'000.0f;
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit) {
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

// Function to get the current crosshair spread multiplier
float AShooterCharacter::GetCrosshairSpreadMultiplier() const {
	return CrosshairSpreadMultiplier;
}

// Function to handle character movement forward
void AShooterCharacter::MoveForward(float Value) {
	if ((Controller != nullptr) && (Value != 0.0f)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

// Function to handle character movement to the right
void AShooterCharacter::MoveRight(float Value) {
	if ((Controller != nullptr) && (Value != 0.0f)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

// Function to handle looking up with the specified rate
void AShooterCharacter::LookUpRate(float Value) {
	// Adjust the pitch of the controller based on input and sensitivity
	APawn::AddControllerPitchInput(Value * CurrentAimSensitivity);
}

// Function to handle turning with the specified rate
void AShooterCharacter::TurnRate(float Value) {
	// Adjust the yaw of the controller based on input and sensitivity
	APawn::AddControllerYawInput(Value * CurrentAimSensitivity);
}

// Function to handle firing the weapon
void AShooterCharacter::FireWeapon() {
	// Play the fire sound if available
	if (FireSound) {
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	// Get the barrel socket for spawning effects
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");

	if (BarrelSocket) {
		// Get the transformation of the barrel socket
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		// Spawn the muzzle flash effect
		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd) {
			// Spawn impact particles at the end of the beam
			if (ImpactParticles) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			// Spawn the beam effect and set its target
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam) {
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}

	// Play the hip-fire animation montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage) {
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	//Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();
}

// Function to get the end location of the beam
bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation) {
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation); //Check for crosshair trace hit

	if (bCrosshairHit) {
		//Tentative beam location - still need to trace under gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else {//No crosshair hit
		//OutBeamLocation is the end location for the line trace
	}
	// Perform a second trace from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd = OutBeamLocation - MuzzleSocketLocation;
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd * 1.25f;
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit) { // Object between barrel and cross-hair
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	} return false;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount) {
	if (OverlappedItemCount + Amount <= 0) {
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else {
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

void AShooterCharacter::TraceForItems() {
	if (bShouldTraceForItems) {
		FHitResult WeaponTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(WeaponTraceResult, HitLocation);
		if (WeaponTraceResult.GetActor()) {
			AItem* HitItem = Cast<AItem>(WeaponTraceResult.GetActor());

			if (HitItem && HitItem->GetPickupWidget()) {
				//Show Item Pickup Widget
				HitItem->GetPickupWidget()->SetVisibility(true);
			}
		}
	}
}