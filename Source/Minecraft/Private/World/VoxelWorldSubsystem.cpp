#include "World/VoxelWorldSubsystem.h"
#include "World/VoxelChunk.h"
#include "World/VoxelWorldSettings.h"
#include "World/WorldGenerationFunctionLibrary.h"


void UVoxelWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called for UVoxelWorldSubsystem"));

	ParseWorldGenerationSettings();
	GenerateNoiseOffset();

	const UVoxelWorldSettings* Settings = GetWorldSettings();
	GeneratePendingChunks(Settings->CenterChunk, Settings->GenerationRadius);
}

void UVoxelWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGenerationEnabled)
	{
		OnGenerationStep.Broadcast(GenerationBatchSize);
		BuildNextBatch();
	}
}

TStatId UVoxelWorldSubsystem::GetStatId() const
{
	// Unreal uses it to identify and measure the time spent ticking this subsystem
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVoxelWorldSubsystem, STATGROUP_Tickables);
}

bool UVoxelWorldSubsystem::IsTickable() const
{
	return !IsTemplate() && IsInitialized() && bGenerationEnabled;
}

void UVoxelWorldSubsystem::Deinitialize()
{
	bGenerationEnabled = false;
	OnGenerationStep.Clear();
	Super::Deinitialize();
}

bool UVoxelWorldSubsystem::TryDestroyBlock(FIntVector GlobalPosition)
{
	if (!CanRemoveBlock(GlobalPosition)) { return false; }

	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);
	const FIntVector BlockLocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	const TObjectPtr<AVoxelChunk>* TargetChunkPtr = Chunks.Find(ChunkIndex);
	if (!TargetChunkPtr || !IsValid(TargetChunkPtr->Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}
	
	AVoxelChunk* TargetChunk = TargetChunkPtr->Get();
	const bool bResult = TargetChunk->SetBlockAtLocalPosition(BlockLocalPosition, EVoxelBlockType::Air);

	return bResult;
}

bool UVoxelWorldSubsystem::TryDestroyBlockFromHit(const FHitResult& Hit, const float TraceEndOffset)
{
	if (!Hit.bBlockingHit) { return false; }

	// Check if the hit actor is a valid voxel chunk
	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor)) { return false; }
	AVoxelChunk* HitChunk = Cast<AVoxelChunk>(HitActor);
	if (!IsValid(HitChunk)) { return false; }

	// Convert the hit location to a global block position
	const FVector HitWorldLocation = Hit.ImpactPoint - Hit.ImpactNormal* TraceEndOffset;
	const FIntVector BlockGlobalPosition = BlockWorldLocationToGlobalPosition(HitWorldLocation);

	if (!CanRemoveBlock(BlockGlobalPosition)) { return false; }

	const FIntVector BlockLocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocal(BlockGlobalPosition, ChunkSize3D);

	const bool bResult = HitChunk->SetBlockAtLocalPositionWithInstanceHint(BlockLocalPosition, EVoxelBlockType::Air, Hit.Item);
	return bResult;
}

bool UVoxelWorldSubsystem::TryPlaceBlock(FIntVector GlobalPosition, EVoxelBlockType Type)
{
	if (!IsWithinBuildLimits(GlobalPosition)) { return false; }

	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);
	const FIntVector BlockLocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	const TObjectPtr<AVoxelChunk>* TargetChunkPtr = Chunks.Find(ChunkIndex);
	if (!TargetChunkPtr || !IsValid(TargetChunkPtr->Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}

	AVoxelChunk* TargetChunk = TargetChunkPtr->Get();
	const bool bResult = TargetChunk->SetBlockAtLocalPosition(BlockLocalPosition, Type);
	return bResult;
}

bool UVoxelWorldSubsystem::GetBlockAtGlobalPosition(FIntVector GlobalPosition, FVoxelBlock& VoxelBlock)
{
	VoxelBlock = FVoxelBlock();
	int32 BlockIndexLocalToChunk = INDEX_NONE;

	if (ChunkSize3D.X <= 0 || ChunkSize3D.Y <= 0)
	{
		return false;
	}

	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);

	const TObjectPtr<AVoxelChunk>* FoundChunk = Chunks.Find(ChunkIndex);
	if (!FoundChunk || !IsValid(FoundChunk->Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}

	const FIntVector LocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocal(GlobalPosition, ChunkSize3D);

	return FoundChunk->Get()->GetBlockAtLocalPosition(LocalPosition, VoxelBlock, BlockIndexLocalToChunk);
}

bool UVoxelWorldSubsystem::SetBlockAtGlobalPosition(FIntVector GlobalPosition, EVoxelBlockType NewType)
{
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);

	const TObjectPtr<AVoxelChunk>* FoundChunk = Chunks.Find(ChunkIndex);
	if (!FoundChunk || !IsValid(FoundChunk->Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}

	const FIntVector LocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	// This chunk function handles data changes AND all affected visuals.
	return FoundChunk->Get()->SetBlockAtLocalPosition(LocalPosition, NewType);
}

FIntVector UVoxelWorldSubsystem::BlockWorldLocationToGlobalPosition(FVector WorldLocation) const
{
	return UWorldGenerationFunctionLibrary::BlockWorldLocationToGlobalPosition(WorldLocation, BlockSize, BlockScale);
}

FVector UVoxelWorldSubsystem::SnapWorldLocationToBlockOrigin(FVector WorldLocation) const
{
	return UWorldGenerationFunctionLibrary::SnapWorldLocationToBlockOrigin(WorldLocation, BlockSize, BlockScale);
}

FVector UVoxelWorldSubsystem::FindSafePlayerSpawnLocation() const
{
	const UVoxelWorldSettings* Settings = GetWorldSettings();
	const FIntPoint CenterChunkIndex2D = Settings->CenterChunk;
	const FIntVector CenterChunkIndex3D(CenterChunkIndex2D.X, CenterChunkIndex2D.Y, 0);

	FVector TargetLocation = UWorldGenerationFunctionLibrary::BlockGlobalPositionToWorldLocation(CenterChunkIndex3D, ChunkSize3D, BlockSize, BlockScale);
	TargetLocation.Z = static_cast<double>(MapHeightRange.Y) * BlockSize.Z * BlockScale.Z;

	return TargetLocation;
}

bool UVoxelWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UVoxelWorldSubsystem::GeneratePendingChunks(FIntPoint CenterChunk, int32 MaximumRadius)
{
	PendingChunks.Reset();

	for (int Radius = 0; Radius <= MaximumRadius; Radius++)
	{
		for (int32 X = -Radius; X <= Radius; ++X)
		{
			for (int32 Y = -Radius; Y <= Radius; ++Y)
			{
				const bool bIsOnBorder =
					FMath::Max(FMath::Abs(X), FMath::Abs(Y)) == Radius;

				if (bIsOnBorder)
				{
					PendingChunks.Add(FIntVector(CenterChunk.X + X, CenterChunk.Y + Y, 0));
				}
			}
		}
	}
}

void UVoxelWorldSubsystem::BuildNextBatch()
{
	if (IsValid(ActiveChunk))
	{
		bool IsGenerationCompleted = ActiveChunk->ProcessGenerationStep(GenerationBatchSize);
		if (IsGenerationCompleted) 
		{
			const FIntVector ChunkIndex = ActiveChunk->GetChunkIndex();
			OnChunkGenerationFinished.Broadcast(ChunkIndex);
			ActiveChunk = nullptr;
		}
	}
	else
	{
		SpawnNextChunk();
	}
}

AVoxelChunk* UVoxelWorldSubsystem::SpawnNewChunk(FIntVector TargetChunkIndex)
{
	if (!IsValid(ChunkClass)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkClass is invalid!"));
		return nullptr;
	}

	FTransform Transform = FTransform(CalculateChunkLocation(TargetChunkIndex));

	AVoxelChunk* NewChunk = GetWorld()->SpawnActor<AVoxelChunk>(ChunkClass, Transform);
	if (!IsValid(NewChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Error occured while trying to spawn a new Chunk object!"));
		return nullptr;
	}

	NewChunk->InitializeChunk(TargetChunkIndex, ChunkSize2D, MapHeightRange, NoiseOffset, NoiseFrequency, BlockScale);

	Chunks.Add(TargetChunkIndex, NewChunk);

	return NewChunk;
}

AVoxelChunk* UVoxelWorldSubsystem::SpawnNextChunk()
{
	if (NextChunkIndex < PendingChunks.Num()) 
	{
		ActiveChunk = SpawnNewChunk(PendingChunks[NextChunkIndex]);
		NextChunkIndex++;
	}
	else 
	{
		// Job finished
		PendingChunks.Empty();
		NextChunkIndex = 0;
		ActiveChunk = nullptr;
		bGenerationEnabled = false;
		OnWorldGenerationFinished.Broadcast();
	}

	return ActiveChunk;
}

FVector UVoxelWorldSubsystem::CalculateChunkLocation(FIntVector ChunkIndex)
{
	FIntVector BlockCountFromZero = ChunkIndex * ChunkSize3D;
	FVector BlockFinalSize = BlockSize * BlockScale;
	return FVector(BlockCountFromZero.X * BlockFinalSize.X, BlockCountFromZero.Y * BlockFinalSize.Y, BlockCountFromZero.Z * BlockFinalSize.Z);
}

const UVoxelWorldSettings* UVoxelWorldSubsystem::GetWorldSettings() const
{
	return GetDefault<UVoxelWorldSettings>();
}

void UVoxelWorldSubsystem::ParseWorldGenerationSettings()
{
	// Load settings from the world settings object. It only provides consistent values for all generated chunks.
	const UVoxelWorldSettings* Settings = GetWorldSettings();

	InitialWorldSizeXY = Settings->InitialWorldSizeXY;
	ChunkSize2D = Settings->ChunkSize;
	ChunkSize3D = FIntVector(ChunkSize2D.X, ChunkSize2D.Y, Settings->MapHeightRange.Y);
	BlockSize = Settings->BlockSize;
	BlockScale = Settings->BlockScale;
	NoiseFrequency = Settings->NoiseFrequency;
	MapHeightRange = Settings->MapHeightRange;
	GenerationBatchSize = Settings->GenerationBatchSize;
	MinBlockZ = Settings->MinBlockZ;
	MaxBlockZ = Settings->MaxBlockZ;

	// ChunkClass is a soft reference in settings.
	ChunkClass = Settings->ChunkClass.LoadSynchronous();
}

void UVoxelWorldSubsystem::RefreshBlockVisualAtGlobalPosition(FIntVector GlobalPosition)
{
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);

	const TObjectPtr<AVoxelChunk>* FoundChunk = Chunks.Find(ChunkIndex);
	if (!FoundChunk || !IsValid(FoundChunk->Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return;
	}

	const FIntVector LocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	FoundChunk->Get()->RefreshBlockVisual(LocalPosition);
}

void UVoxelWorldSubsystem::GenerateNoiseOffset()
{
	// Generate once; every chunk uses this same value.
	NoiseOffset = FVector2D(
		FMath::FRandRange(-10000.0f, 10000.0f),
		FMath::FRandRange(-10000.0f, 10000.0f));
}

bool UVoxelWorldSubsystem::CanRemoveBlock(const FIntVector& Position) const
{
	return Position.Z > MinBlockZ;
}

bool UVoxelWorldSubsystem::IsWithinBuildLimits(const FIntVector& Position) const
{
	return Position.Z <= MaxBlockZ;
}
