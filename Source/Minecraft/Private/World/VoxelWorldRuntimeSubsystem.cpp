#include "World/VoxelWorldRuntimeSubsystem.h"
#include "World/VoxelChunk.h"
#include "World/VoxelWorldSettings.h"
#include "World/WorldGenerationFunctionLibrary.h"

#include "SaveSystem/VoxelWorldSaveSubsystem.h"
#include "SaveSystem/VoxelChunkDelta.h"

#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"


void UVoxelWorldRuntimeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	SaveSubsystem = InWorld.GetSubsystem<UVoxelWorldSaveSubsystem>();

	GeneratePendingChunks(LastPlayerChunkIndex, GenerationRadius);
}

void UVoxelWorldRuntimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OnGenerationStep.Broadcast(GenerationBatchSize);
	BuildNextBatch();
}

TStatId UVoxelWorldRuntimeSubsystem::GetStatId() const
{
	// Unreal uses it to identify and measure the time spent ticking this subsystem
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVoxelWorldRuntimeSubsystem, STATGROUP_Tickables);
}

bool UVoxelWorldRuntimeSubsystem::IsTickable() const
{
	return !IsTemplate() && IsInitialized() && bGenerationEnabled;
}

void UVoxelWorldRuntimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ParseWorldGenerationSettings();
	GenerateNoiseOffset();
}

void UVoxelWorldRuntimeSubsystem::Deinitialize()
{
	bGenerationEnabled = false;
	OnGenerationStep.Clear();

	//CurrentSaveGame->NoiseFrequency = GetNoiseFrequency();
	//CurrentSaveGame->NoiseOffset = GetNoiseOffset();
	GetLastVisitedChunkIndex();


	Super::Deinitialize();
}

bool UVoxelWorldRuntimeSubsystem::TryDestroyBlock(FIntVector GlobalPosition)
{
	if (!CanRemoveBlock(GlobalPosition)) { return false; }

	const FIntVector ChunkSize3D = GenerationSettings.GetChunkSize3D();
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);
	const FIntVector BlockLocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if (!IsValid(TargetChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}
	
	const bool bResult = TargetChunk->SetBlockAtLocalPosition(BlockLocalPosition, EVoxelBlockType::Air);

	return bResult;
}

bool UVoxelWorldRuntimeSubsystem::TryDestroyBlockFromHit(const FHitResult& Hit, const float TraceEndOffset)
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

	const FIntVector BlockLocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocal(BlockGlobalPosition, GenerationSettings.GetChunkSize3D());

	const bool bResult = HitChunk->SetBlockAtLocalPositionWithInstanceHint(BlockLocalPosition, EVoxelBlockType::Air, Hit.Item);
	return bResult;
}

bool UVoxelWorldRuntimeSubsystem::TryPlaceBlock(FIntVector GlobalPosition, EVoxelBlockType Type)
{
	if (!IsWithinBuildLimits(GlobalPosition)) { return false; }

	const FIntVector ChunkSize3D = GenerationSettings.GetChunkSize3D();
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);
	const FIntVector BlockLocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if (!IsValid(TargetChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}

	const bool bResult = TargetChunk->SetBlockAtLocalPosition(BlockLocalPosition, Type);
	return bResult;
}

bool UVoxelWorldRuntimeSubsystem::GetBlockAtGlobalPosition(FIntVector GlobalPosition, FVoxelBlock& VoxelBlock)
{
	VoxelBlock = FVoxelBlock();
	int32 BlockIndexLocalToChunk = INDEX_NONE;

	const FIntVector ChunkSize3D = GenerationSettings.GetChunkSize3D();
	if (ChunkSize3D.X <= 0 || ChunkSize3D.Y <= 0)
	{
		return false;
	}

	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);

	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if (!IsValid(TargetChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}

	const FIntVector LocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocal(GlobalPosition, ChunkSize3D);

	return TargetChunk->GetBlockAtLocalPosition(LocalPosition, VoxelBlock, BlockIndexLocalToChunk);
}

bool UVoxelWorldRuntimeSubsystem::SetBlockAtGlobalPosition(FIntVector GlobalPosition, EVoxelBlockType NewType)
{
	const FIntVector ChunkSize3D = GenerationSettings.GetChunkSize3D();
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize3D);

	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if (!IsValid(TargetChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return false;
	}

	const FIntVector LocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize3D, ChunkIndex);

	// This chunk function handles data changes AND all affected visuals.
	return TargetChunk->SetBlockAtLocalPosition(LocalPosition, NewType);
}

FIntVector UVoxelWorldRuntimeSubsystem::BlockWorldLocationToGlobalPosition(FVector WorldLocation) const
{
	return UWorldGenerationFunctionLibrary::BlockWorldLocationToGlobalPosition(WorldLocation, GenerationSettings.BlockSize, GenerationSettings.BlockScale);
}

FVector UVoxelWorldRuntimeSubsystem::SnapWorldLocationToBlockOrigin(FVector WorldLocation) const
{
	return UWorldGenerationFunctionLibrary::SnapWorldLocationToBlockOrigin(WorldLocation, GenerationSettings.BlockSize, GenerationSettings.BlockScale);
}

FVector UVoxelWorldRuntimeSubsystem::FindSafePlayerSpawnLocation() const
{
	const UVoxelWorldSettings* Settings = GetWorldSettings();
	const FIntPoint CenterChunkIndex2D = Settings->CenterChunk;
	const FIntVector CenterChunkIndex3D(CenterChunkIndex2D.X, CenterChunkIndex2D.Y, 0);

	FVector TargetLocation = UWorldGenerationFunctionLibrary::BlockGlobalPositionToWorldLocation(CenterChunkIndex3D, GenerationSettings.GetChunkSize3D(), GenerationSettings.BlockSize, GenerationSettings.BlockScale);
	TargetLocation.Z = static_cast<double>(GenerationSettings.MapHeightRange.Y) * GenerationSettings.BlockSize.Z * GenerationSettings.BlockScale.Z;

	return TargetLocation;
}

void UVoxelWorldRuntimeSubsystem::ForEachLoadedChunk(TFunctionRef<void(const AVoxelChunk&)> Visitor) const
{
	for (const auto& [Coord, Chunk] : Chunks)
	{
		const AVoxelChunk* TargetChunk = Chunk.Get();
		if (!IsValid(TargetChunk)) { continue; }
		if (!TargetChunk->IsGenerationComplete()) { continue; }
		Visitor(*TargetChunk);
	}
}

AVoxelChunk* UVoxelWorldRuntimeSubsystem::GetChunkByIndex(const FIntVector ChunkIndex) const
{
	return Chunks.FindRef(ChunkIndex);
}

bool UVoxelWorldRuntimeSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

AVoxelChunk* UVoxelWorldRuntimeSubsystem::FindOrCreateChunk(FIntVector ChunkIndex)
{
	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if(IsValid(TargetChunk))
	{
		return TargetChunk;
	}

	TargetChunk = SpawnNewChunk(ChunkIndex);

	return TargetChunk;
}

void UVoxelWorldRuntimeSubsystem::GeneratePendingChunks(FIntVector CenterChunk, int32 MaximumRadius)
{
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
					FIntVector ChunkIndex(CenterChunk.X + X, CenterChunk.Y + Y, CenterChunk.Z);

					AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
					if (IsValid(TargetChunk))
					{
						if (!TargetChunk->MarkGenerationPending()) { continue; }
					}

					PendingChunks.Enqueue(ChunkIndex);
				}
			}
		}
	}

	bGenerationEnabled = true;
}

void UVoxelWorldRuntimeSubsystem::UnloadDepartingChunks(FIntVector PreviousCenter, FIntVector CurrentCenter, int32 KeepRadius)
{
	// Unload columns of chunks
	const int32 DeltaX = CurrentCenter.X - PreviousCenter.X;

	if (DeltaX != 0)
	{
		const int32 OldMinX = PreviousCenter.X - KeepRadius;
		const int32 OldMaxX = PreviousCenter.X + KeepRadius;

		const int32 MinX = DeltaX > 0
			? OldMinX
			: FMath::Max(OldMinX, CurrentCenter.X + KeepRadius + 1);

		const int32 MaxX = DeltaX > 0
			? FMath::Min(OldMaxX, CurrentCenter.X - KeepRadius - 1)
			: OldMaxX;

		for (int32 X = MinX; X <= MaxX; ++X)
		{
			for (int32 Y = PreviousCenter.Y - KeepRadius;
				Y <= PreviousCenter.Y + KeepRadius;
				++Y)
			{
				UnloadChunk(FIntVector(X, Y, PreviousCenter.Z));
			}
		}
	}

	// Unload rows of chunks
	const int32 DeltaY = CurrentCenter.Y - PreviousCenter.Y;

	if (DeltaY != 0)
	{
		const int32 OldMinY = PreviousCenter.Y - KeepRadius;
		const int32 OldMaxY = PreviousCenter.Y + KeepRadius;

		const int32 MinY = DeltaY > 0
			? OldMinY
			: FMath::Max(OldMinY, CurrentCenter.Y + KeepRadius + 1);

		const int32 MaxY = DeltaY > 0
			? FMath::Min(OldMaxY, CurrentCenter.Y - KeepRadius - 1)
			: OldMaxY;

		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			for (int32 X = CurrentCenter.X - KeepRadius;
				X <= CurrentCenter.X + KeepRadius;
				++X)
			{
				UnloadChunk(FIntVector(X, Y, PreviousCenter.Z));
			}
		}
	}
}

void UVoxelWorldRuntimeSubsystem::BuildNextBatch()
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
		FIntVector ChunkIndex;
		const bool IsPendingQueueNotEmpty = PendingChunks.Dequeue(ChunkIndex);

		if (IsPendingQueueNotEmpty)
		{
			ActiveChunk = FindOrCreateChunk(ChunkIndex);
			ActiveChunk->MarkGenerationPending();
			return;
		}

		// Job finished
		PendingChunks.Reset();
		ActiveChunk = nullptr;
		bGenerationEnabled = false;
		OnChunkGenerationQueueDrained.Broadcast();

		if (!bInitialWorldReady)
		{
			bInitialWorldReady = true;
			OnInitialWorldGenerated();
		}
	}
}

AVoxelChunk* UVoxelWorldRuntimeSubsystem::SpawnNewChunk(FIntVector TargetChunkIndex)
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

	NewChunk->InitializeChunk(TargetChunkIndex, GenerationSettings.ChunkSize2D, 
		GenerationSettings.MapHeightRange, GenerationSettings.NoiseOffset, 
		GenerationSettings.NoiseFrequency, GenerationSettings.BlockScale, GenerationSettings.BlockSize);

	Chunks.Add(TargetChunkIndex, NewChunk);

	return NewChunk;
}

FVector UVoxelWorldRuntimeSubsystem::CalculateChunkLocation(FIntVector ChunkIndex)
{
	FIntVector BlockCountFromZero = ChunkIndex * GenerationSettings.GetChunkSize3D();
	FVector BlockFinalSize = GenerationSettings.BlockSize * GenerationSettings.BlockScale;
	return FVector(BlockCountFromZero.X * BlockFinalSize.X, BlockCountFromZero.Y * BlockFinalSize.Y, BlockCountFromZero.Z * BlockFinalSize.Z);
}

const UVoxelWorldSettings* UVoxelWorldRuntimeSubsystem::GetWorldSettings() const
{
	return GetDefault<UVoxelWorldSettings>();
}

void UVoxelWorldRuntimeSubsystem::ParseWorldGenerationSettings()
{
	const UVoxelWorldSettings* Settings = GetWorldSettings();

	// Read settings not saved in the save system
	InitialWorldSizeXY = Settings->InitialWorldSizeXY;
	GenerationBatchSize = Settings->InitialGenerationBatchSize;
	InitialGenerationBatchSize = Settings->InitialGenerationBatchSize;
	GameplayGenerationBatchSize = Settings->GameplayGenerationBatchSize;
	MinBlockZ = Settings->MinBlockZ;
	MaxBlockZ = Settings->MaxBlockZ;
	GenerationRadius = Settings->GenerationRadius;
	LastPlayerChunkIndex = FIntVector(Settings->CenterChunk.X, Settings->CenterChunk.Y, 0);

	// ChunkClass is a soft reference in settings.
	ChunkClass = Settings->ChunkClass.LoadSynchronous();

	// Read settings that can be saved in the save system
	GenerationSettings = Settings->GenerationSettings;

	if (!IsValid(SaveSubsystem)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Save Subsystem is not valid. Generation settings will be default!"));
		return; 
	}

	FVoxelWorldGenerationSettings SavedSettings;
	/*if (SaveSubsystem->TryGetGenerationSettings(SavedSettings))
	{
		GenerationSettings = SavedSettings;
	}
	else
	{
		SaveSubsystem->SetGenerationSettings(GenerationSettings);
	}*/
}

void UVoxelWorldRuntimeSubsystem::RefreshBlockVisualAtGlobalPosition(FIntVector GlobalPosition)
{
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(GlobalPosition, GenerationSettings.GetChunkSize3D());

	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if (!IsValid(TargetChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for GlobalPosition: %s"), *GlobalPosition.ToString());
		return;
	}

	const FIntVector LocalPosition = UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, GenerationSettings.GetChunkSize3D(), ChunkIndex);

	TargetChunk->RefreshBlockVisual(LocalPosition);
}

void UVoxelWorldRuntimeSubsystem::GenerateNoiseOffset()
{
	// Generate once; every chunk uses this same value.
	GenerationSettings.NoiseOffset = FVector2D(
		FMath::FRandRange(-10000.0f, 10000.0f),
		FMath::FRandRange(-10000.0f, 10000.0f));
}

bool UVoxelWorldRuntimeSubsystem::CanRemoveBlock(const FIntVector& Position) const
{
	return Position.Z > MinBlockZ;
}

bool UVoxelWorldRuntimeSubsystem::IsWithinBuildLimits(const FIntVector& Position) const
{
	return Position.Z <= MaxBlockZ;
}

void UVoxelWorldRuntimeSubsystem::CheckPlayerChunk()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!IsValid(PlayerCharacter)) { return; }

	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	const FIntVector BlockPosition = UWorldGenerationFunctionLibrary::BlockWorldLocationToGlobalPosition(PlayerLocation, GenerationSettings.BlockSize, GenerationSettings.BlockScale);
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(BlockPosition, GenerationSettings.GetChunkSize3D());

	if (LastPlayerChunkIndex == ChunkIndex)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Player moved to a new chunk: %s"), *ChunkIndex.ToString());

	// For example, load within 6 chunks and unload beyond 8 chunks. 
	// That gap prevents chunks repeatedly spawning and despawning 
	// when the player walks back and forth along a boundary

	GeneratePendingChunks(ChunkIndex, GenerationRadius);

	UnloadDepartingChunks(LastPlayerChunkIndex, ChunkIndex, GenerationRadius + UnloadMargin);

	LastPlayerChunkIndex = ChunkIndex;
}

bool UVoxelWorldRuntimeSubsystem::UnloadChunk(FIntVector ChunkIndex)
{
	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if (!IsValid(TargetChunk))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for unloading: %s"), *ChunkIndex.ToString());
		return false;
	}

	if (TargetChunk->IsGenerationPending())
	{
		PendingChunks.Cancel(ChunkIndex);
	}

	if (ActiveChunk == TargetChunk)
	{
		ActiveChunk = nullptr;
	}

	OnChunkUnloadingRequested.Broadcast(ChunkIndex);

	CaptureChunk(TargetChunk);

	TargetChunk->CancelGenerationAndClear();
	return true;
}

void UVoxelWorldRuntimeSubsystem::OnInitialWorldGenerated()
{
	OnInitialWorldReady.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		ChunkCheckTimerHandle,
		this,
		&UVoxelWorldRuntimeSubsystem::CheckPlayerChunk,
		ChunkCheckTimerDelta,  // Interval in seconds
		true   // Repeat
	);

	GenerationBatchSize = GameplayGenerationBatchSize;
}

void UVoxelWorldRuntimeSubsystem::CaptureLoadedChunks()
{
	for (const auto& Pair : Chunks)
	{
		CaptureChunk(Pair.Value);
	}
}

void UVoxelWorldRuntimeSubsystem::CaptureChunk(AVoxelChunk* TargetChunk)
{
	if (!IsValid(TargetChunk)) { return; }

	if (!IsValid(SaveSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Save Subsystem is not valid. Chunk will not be saved!"));
		return;
	}

	// Check whether the chunk is properly generated and has unsaved modifications
	if (!TargetChunk->IsDirty() || !TargetChunk->IsGenerationComplete())
	{
		return;
	}

	SaveSubsystem->SaveChunkModifications(TargetChunk->GetChunkIndex(),
		TargetChunk->GetChunkModification());

	// Mark clean after successful capture.
	TargetChunk->SetDirty(false);
}
