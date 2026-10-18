#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

/** Base for Blueprint chunks whose generation is scheduled by the world subsystem. */
UCLASS(Blueprintable)
class MINECRAFT_API AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	/** Store generation settings and reset chunk data/progress */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Generation")
	bool InitializeChunk(FIntPoint ChunkIndex, FIntPoint ChunkSize,
		FIntPoint ChunkHeightRange, FVector2D NoiseOffset,
		float NoiseFrequency, FVector BlockScale);

	/** Process a bounded amount of generation work. Return true when complete. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Generation")
	bool ProcessGenerationStep(int32 BatchSize);
};
