#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VoxelWorldSubsystem.generated.h"

// VoxelWorldSubsystem.h
class AVoxelChunk;
class UVoxelWorldSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxelGenerationStep, int32, BatchSize);

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

	/** Bind a generation handler before enabling generation. Runs on the game thread. */
	UPROPERTY(BlueprintAssignable, Category = "Voxel World|Generation")
	FVoxelGenerationStep OnGenerationStep;

	UFUNCTION(BlueprintPure, Category = "Voxel World|Generation")
	bool IsGenerationEnabled() const { return bGenerationEnabled; }

	UFUNCTION(BlueprintPure, Category = "Voxel World|Generation")
	int32 GetGenerationBatchSize() const { return GenerationBatchSize; }

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	void GeneratePendingChunks(FIntPoint CenterChunk, int32 MaximumRadius);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	void BuildNextBatch();

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	AVoxelChunk* SpawnNewChunk(FIntPoint TargetChunkIndex);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	AVoxelChunk* SpawnNextChunk();

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	FVector CalculateChunkLocation(FIntPoint ChunkIndex);

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Generation")
	const UVoxelWorldSettings* GetWorldSettings();

	UFUNCTION()
	void ParseWorldGenerationSettings();

private:
	UPROPERTY(Transient)
	bool bGenerationEnabled = true;

	UPROPERTY(Transient)
	int32 GenerationBatchSize = 100;

	UPROPERTY(Transient)
	FIntPoint WorldSize = FIntPoint(1,1);

	UPROPERTY(Transient)
	TArray<FIntPoint> PendingChunks;

	UPROPERTY(Transient)
	TObjectPtr<AVoxelChunk> ActiveChunk = nullptr;

	UPROPERTY(Transient)
	int32 NextChunkIndex = 0;

	UPROPERTY(Transient)
	FIntPoint ChunkSize = FIntPoint(1, 1);

	UPROPERTY(Transient)
	FVector BlockSize = FVector::One();

	UPROPERTY(Transient)
	FVector BlockScale = FVector::One();

	UPROPERTY(Transient)
	TSubclassOf<AVoxelChunk> ChunkClass;

	UPROPERTY(Transient)
	FIntPoint MapHeightRange = FIntPoint(1, 1);

	UPROPERTY(Transient)
	FVector2D NoiseOffset;

	UPROPERTY(Transient)
	float NoiseFrequency = 0.05;
};
