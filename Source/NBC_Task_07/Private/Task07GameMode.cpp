
#include "Task07GameMode.h"
#include "PlayerPawn.h"
#include "Task07PlayerController.h"

ATask07GameMode::ATask07GameMode()
{
	// 기본 Pawn 클래스를 APlayerPawn으로 설정
	DefaultPawnClass = APlayerPawn::StaticClass();
    // 기본 PlayerController 클래스를 ATask07PlayerController로 설정
    PlayerControllerClass = ATask07PlayerController::StaticClass();
}
