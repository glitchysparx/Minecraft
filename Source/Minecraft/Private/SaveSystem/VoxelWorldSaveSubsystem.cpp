#include "SaveSystem/VoxelWorldSaveSubsystem.h"
#include "World/VoxelWorldRuntimeSubsystem.h"
#include "SaveSystem/VoxelWorldSaveGame.h"
#include "SaveSystem/VoxelChunkDelta.h"
#include "World/VoxelChunk.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UVoxelWorldSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    if (!LoadWorldDataContainer(CurrentSlotName))
    {
        CurrentSaveGame = CreateNewWorldDataContainer();
    }
}

void UVoxelWorldSaveSubsystem::Deinitialize()
{
    SaveWorldDataContainer();
	Super::Deinitialize();
}

bool UVoxelWorldSaveSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UVoxelWorldSaveSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
}

UVoxelWorldSaveGame* UVoxelWorldSaveSubsystem::CreateNewWorldDataContainer()
{
    UVoxelWorldSaveGame* SaveGame = Cast<UVoxelWorldSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UVoxelWorldSaveGame::StaticClass()));

    return SaveGame;
}

bool UVoxelWorldSaveSubsystem::LoadWorldDataContainer(const FString& SlotName)
{
    UVoxelWorldSaveGame* LoadedSave = Cast<UVoxelWorldSaveGame>
                                (UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!LoadedSave) { return false; }

    CurrentSaveGame = LoadedSave;
    CurrentSlotName = SlotName;

    return true;
}

bool UVoxelWorldSaveSubsystem::SaveWorldDataContainer()
{
    if (!CurrentSaveGame || CurrentSlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot save world data: No current SaveGame or SlotName is empty."));
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSlotName, 0);
}

void UVoxelWorldSaveSubsystem::SaveChunkModifications(const FIntVector ChunkIndex, const FVoxelChunkDelta& Delta)
{
    if (!IsValid(CurrentSaveGame)) 
    { 
		UE_LOG(LogTemp, Warning, TEXT("Cannot save chunk delta: CurrentSaveGame is not valid."));
        return; 
    }

    CurrentSaveGame->SaveChunkDelta(ChunkIndex, Delta);
}

bool UVoxelWorldSaveSubsystem::TryGetSavedChunkModifications(const FIntVector ChunkIndex, FVoxelChunkDelta& OutDelta) const
{
    if (!IsValid(CurrentSaveGame)) 
    { 
		UE_LOG(LogTemp, Warning, TEXT("Cannot retrieve chunk delta: CurrentSaveGame is not valid."));
        return false; 
    }

	return CurrentSaveGame->ReadChunkDelta(ChunkIndex, OutDelta);
}

