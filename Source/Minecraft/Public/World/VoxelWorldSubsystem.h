#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VoxelBlock.h"
#include "VoxelWorldSubsystem.generated.h"

// VoxelWorldSubsystem.h
class AVoxelChunk;
class UVoxelWorldSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxelGenerationStep, int32, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChunkGeneration, FIntVector, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWorldGeneration);

/** World-scoped entry point for voxel world generation and chunk management. */
UCLASS(BlueprintType)
class MINECRAFT_API UVoxelWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FVoxelGenerationStep OnGenerationStep;

	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FChunkGeneration OnChunkGenerationFinished;

	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FWorldGeneration OnWorldGenerationFinished;

	UFUNCTION(BlueprintPure, Category = "Voxel World|Generation")
	bool IsGenerationEnabled() const { return bGenerationEnabled; }

	UFUNCTION(BlueprintPure, Category = "Voxel World|Generation")
	int32 GetGenerationBatchSize() const { return GenerationBatchSize; }

	// Attempts to destroy a block at the specified global position in the voxel world.
	// Standard method for block destruction, but is less optimized than TryDestroyBlockFromHit.
	UFUNCTION(BlueprintCallable, Category = "Voxel World|Editing")
	bool TryDestroyBlock(FIntVector GlobalPosition);

	// Attempts to destroy a block based on a hit result, 
	// which is typically obtained from a line trace or collision detection in the game world.
	// More optimized than TryDestroyBlock
	UFUNCTION(BlueprintCallable, Category = "Voxel World|Editing")
	bool TryDestroyBlockFromHit(const FHitResult& Hit, const float TraceEndOffset = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Editing")
	bool TryPlaceBlock(FIntVector GlobalPosition, EVoxelBlockType Type);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Editing")
	bool GetBlockAtGlobalPosition(FIntVector GlobalPosition, FVoxelBlock& VoxelBlock);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Editing")
	bool SetBlockAtGlobalPosition(FIntVector GlobalPosition, EVoxelBlockType NewType);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Math")
	FIntVector BlockWorldLocationToGlobalPosition(FVector WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	int32 GetPendingChunksCount() const 
	{
		return PendingChunks.Num();
	}

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION()
	void GeneratePendingChunks(FIntPoint CenterChunk, int32 MaximumRadius);

	UFUNCTION()
	void BuildNextBatch();

	UFUNCTION()
	AVoxelChunk* SpawnNewChunk(FIntVector TargetChunkIndex);

	UFUNCTION()
	AVoxelChunk* SpawnNextChunk();

	UFUNCTION()
	FVector CalculateChunkLocation(FIntVector ChunkIndex);

	UFUNCTION()
	const UVoxelWorldSettings* GetWorldSettings();

	UFUNCTION()
	void ParseWorldGenerationSettings();

	UFUNCTION()
	void RefreshBlockVisualAtGlobalPosition(FIntVector GlobalPosition);

	void GenerateNoiseOffset();

	bool CanRemoveBlock(const FIntVector& Position) const;
	bool IsWithinBuildLimits(const FIntVector& Position) const;

private:
	bool bGenerationEnabled = true;
	int32 GenerationBatchSize = 100;

	UPROPERTY(Transient)
	FIntPoint InitialWorldSizeXY;

	UPROPERTY(Transient)
	TArray<FIntVector> PendingChunks;

	UPROPERTY(Transient)
	TMap<FIntVector, TObjectPtr<AVoxelChunk>> Chunks;

	UPROPERTY(Transient)
	TObjectPtr<AVoxelChunk> ActiveChunk = nullptr;

	int32 NextChunkIndex = 0;

	UPROPERTY(Transient)
	FIntPoint ChunkSize2D;

	UPROPERTY(Transient)
	FIntVector ChunkSize3D;

	FVector BlockSize = FVector::One();
	FVector BlockScale = FVector::One();

	UPROPERTY(Transient)
	TSubclassOf<AVoxelChunk> ChunkClass;

	FIntPoint MapHeightRange;

	FVector2D NoiseOffset;
	float NoiseFrequency = 0.05;

	int32 MinBlockZ = 0;
	int32 MaxBlockZ = 0;
};
