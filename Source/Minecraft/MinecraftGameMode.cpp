// Copyright Epic Games, Inc. All Rights Reserved.

#include "MinecraftGameMode.h"
#include "MinecraftCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMinecraftGameMode::AMinecraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
