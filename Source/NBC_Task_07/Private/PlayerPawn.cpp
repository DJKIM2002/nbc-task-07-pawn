// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// CapsuleComponent를 루트로 설정
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	RootComponent = CapsuleComp;
	CapsuleComp->SetCapsuleHalfHeight(88.0f);
	CapsuleComp->SetCapsuleRadius(34.0f);
	
	// CapsuleComponent Collision 설정
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	
	// CapsuleComponent Physics 시뮬레이션 비활성화
	if (CapsuleComp->IsSimulatingPhysics())
	{
		CapsuleComp->SetSimulatePhysics(false);
	}

	// SkeletalMeshComponent
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f)); // Capsule 하단에 맞춤
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // 메시 방향 조정
	
	// MeshComponent Collision 설정 (충돌 비활성화 - CapsuleComp가 담당)
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// MeshComponent Physics 시뮬레이션 비활성화
	if (MeshComp->IsSimulatingPhysics())
	{
		MeshComp->SetSimulatePhysics(false);
	}

	// SpringArmComponent
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	// 컨트롤러 회전에 따라 스프링 암도 회전하도록 설정
	SpringArmComp->bUsePawnControlRotation = true;

	// CameraComponent
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// 스프링 암의 소켓 위치에 카메라를 부착
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	// 카메라는 스프링 암의 회전을 따르므로 PawnControlRotation은 꺼둠
	CameraComp->bUsePawnControlRotation = false;

	MaxWalkSpeed = 500.0f;
	LookSensitivity = 1.0f;
	RotationSpeed = 10.0f;
	CurrentVelocity = FVector::ZeroVector;
	PreviousLocation = FVector::ZeroVector;
	PreviousVelocity = FVector::ZeroVector;
	CurrentAcceleration = FVector::ZeroVector;
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 위치 저장
	PreviousLocation = GetActorLocation();

	// Enhanced Input 서브시스템에 Input Mapping Context 추가
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 속도 및 가속도 업데이트
	UpdateVelocity(DeltaTime);
	
	// Pawn 회전 업데이트
	UpdateRotation(DeltaTime);
}

void APlayerPawn::UpdateVelocity(float DeltaTime)
{
	if (DeltaTime > 0.0f)
	{
		// 현재 위치와 이전 위치의 차이로 속도 계산
		FVector CurrentLocation = GetActorLocation();
		CurrentVelocity = (CurrentLocation - PreviousLocation) / DeltaTime;
		
		// 가속도 계산
		CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;
		
		// 이전 값 저장
		PreviousLocation = CurrentLocation;
		PreviousVelocity = CurrentVelocity;
	}
}

void APlayerPawn::UpdateRotation(float DeltaTime)
{
	// 이동 중일 때만 회전
	if (CurrentVelocity.SizeSquared() > 1.0f)
	{
		// 이동 방향으로 회전
		FVector Direction = CurrentVelocity;
		Direction.Z = 0.0f;
		Direction.Normalize();

		// 목표 회전값 계산
		FRotator TargetRotation = Direction.Rotation();
		
		// 현재 회전에서 목표 회전으로 부드럽게 보간
		FRotator CurrentRotation = GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
		
		// Pawn 회전 설정 (Pitch와 Roll은 0으로 유지)
		SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
	}
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input Component로 캐스팅
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move 액션 바인딩
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerPawn::Move);

		// Look 액션 바인딩
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerPawn::Look);
	}
}

void APlayerPawn::Move(const FInputActionValue& Value)
{
	// X = Forward/Backward, Y = Right/Left
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// 컨트롤러 유효성 검사
	if (!Controller)
	{
		return;
	}

	// 입력이 없으면 리턴
	if (MovementVector.IsNearlyZero())
	{
		return;
	}

	// 컨트롤러 회전에서 Yaw만 가져와서 이동 방향 계산
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	// 이동 방향 벡터 계산
	FVector MovementDirection = FVector::ZeroVector;

	// Forward 방향
	if (MovementVector.X != 0.0f)
	{
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		MovementDirection += ForwardDirection * MovementVector.X;
	}

	// Right 방향
	if (MovementVector.Y != 0.0f)
	{
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		MovementDirection += RightDirection * MovementVector.Y;
	}

	// 이동 방향 정규화 (대각선 이동 시 속도가 빨라지는 것 방지)
	if (!MovementDirection.IsNearlyZero())
	{
		MovementDirection.Normalize();

		// 실제 이동 벡터 계산
		const FVector Movement = MovementDirection * MaxWalkSpeed * GetWorld()->GetDeltaSeconds();
		
		// 충돌 감지를 위한 HitResult
		FHitResult HitResult;
		
		// 이동 시도
		AddActorWorldOffset(Movement, true, &HitResult);

		// 충돌 발생 시 벽을 따라 슬라이딩
		if (HitResult.bBlockingHit)
		{
			// 충돌 지점의 Normal 벡터에 수직인 방향으로 슬라이딩
			FVector Normal = HitResult.Normal;
			Normal.Z = 0.0f; // 평면상에서만 슬라이딩
			Normal.Normalize();

			// 이동 방향을 벽에 평행하게 투영
			const FVector SlideDirection = FVector::VectorPlaneProject(Movement, Normal);
			
			// 슬라이딩 이동 시도
			if (!SlideDirection.IsNearlyZero())
			{
				FHitResult SlideHitResult;
				AddActorWorldOffset(SlideDirection, true, &SlideHitResult);
			}
		}
	}
}

void APlayerPawn::Look(const FInputActionValue& Value)
{
	// X = 마우스 좌우 이동 (Yaw 회전에 사용)
	// Y = 마우스 상하 이동 (Pitch 회전에 사용)
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 컨트롤러 유효성 검사
	if (!Controller)
	{
		return;
	}

	if (LookAxisVector.X != 0.0f || LookAxisVector.Y != 0.0f)
	{
		FRotator NewRotation = Controller->GetControlRotation();

		// Yaw 회전 (좌우 회전) - 컨트롤러 회전 (카메라가 회전)
		NewRotation.Yaw += LookAxisVector.X * LookSensitivity;

		// Pitch 회전 (상하 회전) - 컨트롤러 회전 (-80도 ~ 80도로 제한)
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + (LookAxisVector.Y * LookSensitivity), -80.0f, 80.0f);

		// 컨트롤러 회전 설정
		Controller->SetControlRotation(NewRotation);
	}
}

