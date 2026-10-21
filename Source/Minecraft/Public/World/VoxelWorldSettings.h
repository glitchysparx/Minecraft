#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
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

	/** Number of block cells along each horizontal chunk axis. */
	UPROPERTY(Config, EditAnywhere, Category = "Chunks", meta = (ClampMin = "1"))
	FIntPoint ChunkSize = FIntPoint(16, 16);

	/** Unscaled block dimensions in Unreal units; used for grid spacing. */
	UPROPERTY(Config, EditAnywhere, Category = "Chunks", meta = (ClampMin = "0.001"))
	FVector BlockSize = FVector::OneVector;

	UPROPERTY(Config, EditAnywhere, Category = "Chunks", meta = (ClampMin = "0.001"))
	FVector BlockScale = FVector::OneVector;

	/** Minimum (X) and maximum (Y) terrain elevation in block cells. */
	UPROPERTY(Config, EditAnywhere, Category = "Terrain")
	FIntPoint MapHeightRange = FIntPoint(1, 32);

	UPROPERTY(Config, EditAnywhere, Category = "Terrain", meta = (ClampMin = "0.000001"))
	float NoiseFrequency = 0.05f;

	/** Maximum number of cells/instances processed by one generation step. */
	UPROPERTY(Config, EditAnywhere, Category = "Generation", meta = (ClampMin = "1"))
	int32 GenerationBatchSize = 100;

	UPROPERTY(Config, EditAnywhere, Category = "Chunks", meta = (ClampMin = "1"))
	FIntPoint CenterChunk = FIntPoint(8, 8);

	/** Number of square rings around the center; zero generates only the center. */
	UPROPERTY(Config, EditAnywhere, Category = "Generation", meta = (ClampMin = "0"))
	int32 GenerationRadius = 4;
};
