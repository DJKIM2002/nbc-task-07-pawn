
#include "Task07GameMode.h"
#include "PlayerPawn.h"

ATask07GameMode::ATask07GameMode()
{
	// 기본 Pawn 클래스를 APlayerPawn으로 설정
	DefaultPawnClass = APlayerPawn::StaticClass();
}
