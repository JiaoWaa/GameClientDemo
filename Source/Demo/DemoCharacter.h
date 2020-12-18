// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DemoCharacter.generated.h"

class UAnimMontage;

UCLASS(config=Game)
class ADemoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	ADemoCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */

	//void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */

	//void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

private:
	void Rebirth();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	UPROPERTY(Replicated)
	class ASWeapon* CurrentWeapon;

	//UPROPERTY(EditDefaultsOnly, Category = "Player")
	//TSubclassOf<class ASWeapon> StartWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	//UFUNCTION(BlueprintNativeEvent, Category = "Player")
	//void Fire();

	virtual void BeginPlay() override;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	UPROPERTY(ReplicatedUsing=OnRep_Goal, BlueprintReadOnly, Category = "Player")
	int32 Goal;

	UFUNCTION()
	void OnRep_Goal();

	void OnGoalUpdate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USHealthComponent* HealthComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwnerHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bIsDead;

	FTimerHandle TimerHandleRebirth;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual FVector GetPawnViewLocation() const override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetGoal();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetGoalBlueprintVersion(int32 LastTimeGoal);

	UFUNCTION(Server, Reliable)
	void HandleFire();

	UFUNCTION(BlueprintCallable)
	void ThrowMontage(UAnimMontage* ThrowAnim);

	UFUNCTION(Server, Reliable, WithValidation)
	void ThrowMontageServer(UAnimMontage* ThrowAnim);

	UFUNCTION(NetMulticast, Reliable)
	void ThrowMontageMulticast(UAnimMontage* ThrowAnim);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	class UAnimMontage* ThrowMontageAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player")
	TSubclassOf<class ASWeapon> StartWeaponClass;

	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon();

	virtual bool Die(float KillingDamage, const class UDamageType* DamageType, class AController* Killer, class AActor* DamageCauser);

	UPROPERTY(BlueprintReadOnly, Replicated)
	FRotator ControllerRotation;

	UFUNCTION(BlueprintCallable)
	void SetControllerRotation();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Fire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

};

