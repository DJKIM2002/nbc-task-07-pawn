// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class NBC_TASK_07_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	APlayerPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
#pragma region Components
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* MeshComp;
#pragma endregion
	
#pragma region Camera	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float LookSensitivity;
#pragma endregion

#pragma region Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;
#pragma endregion

#pragma region Movement
	// 이동 속력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxWalkSpeed;

	// 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationSpeed;

	// 현재 이동 속도 벡터
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector CurrentVelocity;

	// 이전 프레임 위치
	FVector PreviousLocation;

	// 이전 프레임 속도
    FVector PreviousVelocity;

    // 현재 가속도
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector CurrentAcceleration;
#pragma endregion

private:
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);

	// 속도 업데이트 함수
	void UpdateVelocity(float DeltaTime);
	
	// 회전 업데이트 함수
	void UpdateRotation(float DeltaTime);
};
