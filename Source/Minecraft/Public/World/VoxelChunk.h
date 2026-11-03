#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelBlock.h"
#include "VoxelChunk.generated.h"


USTRUCT(BlueprintType)
struct FPendingBlockInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BlockIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform = FTransform::Identity;
};


UENUM(BlueprintType)
enum class EChunkGenerationState : uint8
{
	Empty,
	Pending,
	GenerateData,
	FindExposedBlocks,
	BuildingInstances,
	Completed
};


/** Base for Blueprint chunks whose generation is scheduled by the world subsystem. */
UCLASS(Blueprintable)
class MINECRAFT_API AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	/** Store generation settings and reset chunk data/progress */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Generation")
	bool InitializeChunk (	FIntVector Index, FIntPoint Size,
							FIntPoint HeightRange, FVector2D NoiseOffset,
							float NoiseFrequency, FVector BlockScale );

	/** Process a bounded amount of generation work. Return true when complete. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Generation")
	bool ProcessGenerationStep(int32 BatchSize);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Editing")
	bool GetBlockAtLocalPosition(FIntVector LocalPosition, FVoxelBlock& VoxelBlock, int32& LocalBlockIndex);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Editing")
	bool SetBlockAtLocalPosition(FIntVector LocalPosition, EVoxelBlockType NewType);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Editing")
	bool SetBlockAtLocalPositionWithInstanceHint (	FIntVector LocalPosition, EVoxelBlockType NewType,
													int32 InstanceIndexHint = -1 );

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Voxel Chunk|Editing")
	bool RefreshBlockVisual(FIntVector LocalPosition);

	UFUNCTION(BlueprintCallable, Category = "Voxel Chunk|Chunk Data")
	FIntVector GetChunkIndex() const { return ChunkIndex; }

	UFUNCTION(BlueprintCallable, Category = "Voxel Chunk|Chunk Data")
	bool IsGenerationInProgress() const 
	{ 
		return	GenerationState != EChunkGenerationState::Completed 
				&& GenerationState != EChunkGenerationState::Pending;
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel Chunk|Chunk Data")
	bool IsGenerationComplete() const 
	{ 
		return GenerationState == EChunkGenerationState::Completed; 
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel Chunk|Chunk Data")
	EChunkGenerationState GetGenerationState() const { return GenerationState; }

	UFUNCTION(BlueprintCallable, Category = "Voxel Chunk|Chunk Data")
	bool IsGenerationPending() const 
	{ 
		return GenerationState == EChunkGenerationState::Pending; 
	}

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (ForceAsFunction), Category = "Voxel Chunk|Generation")
	void CancelGenerationAndClear();

	bool MarkGenerationPending();

protected:
	UFUNCTION(BlueprintCallable, Category = "Voxel Chunk|Chunk Data")
	void SetGenerationState(EChunkGenerationState NewState) 
	{
		GenerationState = NewState;
	}

	UPROPERTY(BlueprintReadWrite)
	TArray<FVoxelBlock> Blocks;

	UPROPERTY(BlueprintReadWrite)
	FIntVector ChunkIndex = FIntVector::ZeroValue;

	UPROPERTY(BlueprintReadWrite)
	EChunkGenerationState GenerationState = EChunkGenerationState::Pending;
};
