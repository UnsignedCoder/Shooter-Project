// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AShooterCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    /* Called for forward and backward movement */
    void MoveForward(float Value);

	//Mouse and controller camera controls
    void LookUpRate(float Value);
    void TurnRate(float Value);

    /* Strafe movement */
    void MoveRight(float Value);


    void FireWeapon(); // Called when the fire button is clicked

    bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

    // Set bAiming to true or false with button press
    void AimingButtonPressed();
    void AimingButtonReleased();

    void FireButtonPressed();
    void FireButtonReleased();

    void StartFireTimer();

    UFUNCTION()
    void AutoFireReset();

    // Aim Sensitivity Setting When Aiming
    void SensitivitySetting();

    // Interpolate camera zoom FOV
    void CameraInterpZoom(float DeltaTime);

    // Calculate crosshair spread based on various factors
    void CalculateCrosshairSpread(float DeltaTime);

    void StartCrosshairBulletFire();

    UFUNCTION()
    void FinishCrosshairBulletFire();

    //Line trace for items under the crosshair
    bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    /* Positions the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    /* Camera that follows the camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // True when aiming
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bAiming;

    // Default camera FOV
    float CameraDefaultFOV;

    // Camera Zoomed FOV
    float CameraZoomedFOV;

    // Current Field Of View (FOV)
    float CameraCurrentFOV;

    // Interpolation speed for zooming when aiming
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float ZoomInterpSpeed;

    // Mouse and controller aim sensitivity for hip-fire
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = Camera,
        meta = (AllowPrivateAccess = "true"),
        meta = (ClampMin = "0.01", ClampMax = "2.0", UIMin = "0.01", UIMax = "2.0"))
    float HipFireSensitivity;

    // Mouse and controller aim sensitivity when aiming down sights (ADS)
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = Camera,
        meta = (AllowPrivateAccess = "true"),
        meta = (ClampMin = "0.01", ClampMax = "2.0", UIMin = "0.01", UIMax = "2.0"))
    float ADSSensitivity;

    // Active Aim Sensitivity (either HipFireSensitivity or ADSSensitivity)
    float CurrentAimSensitivity;

    // Randomized gunshot sound cue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USoundCue* FireSound;

    // Flash Spawned at Barrel Socket
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* MuzzleFlash;

    // Montage for weapon fire
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* HipFireMontage;

    // Particles spawned on bullet impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticles;

    // Smoke Trail for bullets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* BeamParticles;

    // Determines the spread of the crosshair
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairSpreadMultiplier;

    // Velocity component for crosshair spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairVelocityFactor;

    // Air component for crosshair spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairInAirFactor;

    // Aiming component for crosshair spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairAimingFactor;

    // Shooting component for crosshair spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairShootingFactor;

    float ShootTimeDuration;

    bool bFiringBullet;

    FTimerHandle CrosshairShootTimer;

    //Left mouse button pressed or right controller trigger pressed 
    bool bFireButtonPressed;

    //True when can fire and false when waiting for the timer
    bool bShouldFire;

    //Rate of auto fire
    float AutomaticFireRate;

    //Sets a timer between Gunshots
    FTimerHandle AutoFireTImer;

    //True if should trace every frame for items
    bool bShouldTraceForItems;

public:
    /* Returns camera boom sub-object */
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    /* Returns follow camera sub-object */
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // Returns whether the character is aiming or not
    FORCEINLINE bool GetAiming() const { return bAiming; }

    // Blueprint callable function to get crosshair spread multiplier
    UFUNCTION(BlueprintCallable)
    float GetCrosshairSpreadMultiplier() const;
};
