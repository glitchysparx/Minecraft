#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VoxelBlock.h"
#include "PendingChunkQueue.h"
#include "VoxelWorldGenerationSettings.h"
#include "VoxelWorldRuntimeSubsystem.generated.h"

class AVoxelChunk;
class UVoxelWorldSettings;
class UVoxelWorldSaveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxelGenerationStep, int32, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChunkOperation, FIntVector, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWorldGeneration);

/** World-scoped entry point for voxel world generation and chunk management. */
UCLASS(BlueprintType)
class MINECRAFT_API UVoxelWorldRuntimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FVoxelGenerationStep OnGenerationStep;

	// Every time an individual chunk completes generation
	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FChunkOperation OnChunkGenerationFinished;

	// Every time an individual chunk is requested to be unloaded (but not started unloading yet)
	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FChunkOperation OnChunkUnloadingRequested;

	// Every time the generation system finishes its current workload
	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FWorldGeneration OnChunkGenerationQueueDrained;

	// Once when the required initial spawn region becomes ready
	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FWorldGeneration OnInitialWorldReady;

	UFUNCTION(BlueprintPure, Category = "Voxel World|Generation")
	bool IsGenerationEnabled() const { return bGenerationEnabled; }

	/*UFUNCTION(BlueprintPure, Category = "Voxel World|Generation")
	int32 GetGenerationBatchSize() const { return GenerationBatchSize; }*/

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

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Math")
	FVector SnapWorldLocationToBlockOrigin(FVector WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	int32 GetPendingChunksCount() const 
	{
		return PendingChunks.Num();
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	FVector FindSafePlayerSpawnLocation() const;

	void ForEachLoadedChunk(TFunctionRef<void(const AVoxelChunk&)> Visitor) const;
	AVoxelChunk* GetChunkByIndex(const FIntVector ChunkIndex) const;

	FIntVector GetLastVisitedChunkIndex() const { return LastPlayerChunkIndex; }

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	
	AVoxelChunk* FindOrCreateChunk(FIntVector ChunkIndex);

	UFUNCTION()
	void GeneratePendingChunks(FIntVector CenterChunk, int32 MaximumRadius);

	UFUNCTION()
	void UnloadDepartingChunks(FIntVector PreviousCenter, FIntVector CurrentCenter, int32 KeepRadius);

	UFUNCTION()
	void BuildNextBatch();

	UFUNCTION()
	AVoxelChunk* SpawnNewChunk(FIntVector TargetChunkIndex);

	UFUNCTION()
	FVector CalculateChunkLocation(FIntVector ChunkIndex);

	UFUNCTION()
	const UVoxelWorldSettings* GetWorldSettings() const;

	UFUNCTION()
	void ParseWorldGenerationSettings();

	UFUNCTION()
	void RefreshBlockVisualAtGlobalPosition(FIntVector GlobalPosition);

	void GenerateNoiseOffset();

	bool CanRemoveBlock(const FIntVector& Position) const;
	bool IsWithinBuildLimits(const FIntVector& Position) const;

	void CheckPlayerChunk();

	UFUNCTION()
	bool UnloadChunk(FIntVector ChunkIndex);

	void OnInitialWorldGenerated();

	void CaptureLoadedChunks();
	void CaptureChunk(AVoxelChunk* TargetChunk);

private:
	FVoxelWorldGenerationSettings GenerationSettings;

	bool bInitialWorldReady = false;

	bool bGenerationEnabled = true;
	int32 GenerationBatchSize = 100;

	int32 InitialGenerationBatchSize = 100;
	int32 GameplayGenerationBatchSize = 100;

	UPROPERTY(Transient)
	FIntPoint InitialWorldSizeXY;

	FPendingChunkQueue PendingChunks;

	UPROPERTY(Transient)
	TMap<FIntVector, TObjectPtr<AVoxelChunk>> Chunks;

	UPROPERTY(Transient)
	TObjectPtr<AVoxelChunk> ActiveChunk = nullptr;

	int32 GenerationRadius = 0;
	int32 UnloadMargin = 2;

	UPROPERTY(Transient)
	TSubclassOf<AVoxelChunk> ChunkClass;

	int32 MinBlockZ = 0;
	int32 MaxBlockZ = 0;

	FTimerHandle ChunkCheckTimerHandle;
	const float ChunkCheckTimerDelta = 1.f; // in seconds

	FIntVector LastPlayerChunkIndex = FIntVector::ZeroValue;

	UVoxelWorldSaveSubsystem* SaveSubsystem = nullptr;
};
