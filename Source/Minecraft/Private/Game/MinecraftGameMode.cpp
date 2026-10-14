// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/MinecraftGameMode.h"
#include "Player/MinecraftCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMinecraftGameMode::AMinecraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
