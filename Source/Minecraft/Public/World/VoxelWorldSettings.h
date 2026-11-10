#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VoxelWorldGenerationSettings.h"
#include "VoxelWorldSettings.generated.h"

class AVoxelChunk;

/** Project defaults. Per-world saves and runtime progress belong outside this settings object. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Voxel World"))
class MINECRAFT_API UVoxelWorldSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** Number of block cells along each horizontal chunk axis. */
	UPROPERTY(Config, EditAnywhere, Category = "Chunks", meta = (ClampMin = "1"))
	FIntPoint InitialWorldSizeXY = FIntPoint(16, 16);

	/** Blueprint Actor class used for spawned chunks. */
	UPROPERTY(Config, EditAnywhere, Category = "Chunks")
	TSoftClassPtr<AVoxelChunk> ChunkClass;

	/** Maximum number of cells/instances processed by one generation step during initial world generation process*/
	UPROPERTY(Config, EditAnywhere, Category = "Generation", meta = (ClampMin = "1"))
	int32 InitialGenerationBatchSize = 1000;

	/** Maximum number of cells/instances processed by one generation step during gameplay*/
	UPROPERTY(Config, EditAnywhere, Category = "Generation", meta = (ClampMin = "1"))
	int32 GameplayGenerationBatchSize = 300;

	UPROPERTY(Config, EditAnywhere, Category = "Chunks", meta = (ClampMin = "1"))
	FIntPoint CenterChunk = FIntPoint(8, 8);

	/** Number of square rings around the center; zero generates only the center. */
	UPROPERTY(Config, EditAnywhere, Category = "Generation", meta = (ClampMin = "0"))
	int32 GenerationRadius = 4;

	// Blocks at this level cannot be destroyed.
	UPROPERTY(EditAnywhere, Config, Category = "World Limits")
	int32 MinBlockZ = 0;

	// Highest level where a block can be placed.
	UPROPERTY(EditAnywhere, Config, Category = "World Limits")
	int32 MaxBlockZ = 63;

	UPROPERTY(EditAnywhere, Config, Category = "Generation")
	FVoxelWorldGenerationSettings GenerationSettings;
};
