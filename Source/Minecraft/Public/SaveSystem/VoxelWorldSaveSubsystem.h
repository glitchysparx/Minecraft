#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VoxelWorldSaveSubsystem.generated.h"

class UVoxelWorldSaveGame;
struct FVoxelChunkDelta;

UCLASS()
class MINECRAFT_API UVoxelWorldSaveSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // UWorldSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    UVoxelWorldSaveGame* CreateNewWorldDataContainer();
    bool LoadWorldDataContainer(const FString& SlotName);
    bool SaveWorldDataContainer();

    UVoxelWorldSaveGame* GetCurrentSaveGame() const
    {
		return CurrentSaveGame;
    }

    UFUNCTION()
    void SaveChunkModifications(const FIntVector ChunkIndex, const FVoxelChunkDelta& Delta);

    UFUNCTION()
	bool TryGetSavedChunkModifications(const FIntVector ChunkIndex, FVoxelChunkDelta& OutDelta) const;

    /*UFUNCTION()
    void SetGenerationSettings(const FVoxelWorldGenerationSettings& Settings);

    UFUNCTION()
    bool TryGetGenerationSettings(FVoxelWorldGenerationSettings& OutSettings) const;*/

private:
    UPROPERTY(Transient)
    TObjectPtr<UVoxelWorldSaveGame> CurrentSaveGame = nullptr;

    FString CurrentSlotName = TEXT("TestWorld");;
};
