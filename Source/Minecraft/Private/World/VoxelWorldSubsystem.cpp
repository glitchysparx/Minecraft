#include "World/VoxelWorldSubsystem.h"
#include "World/VoxelChunk.h"
#include "World/VoxelWorldSettings.h"
#include "World/WorldGenerationFunctionLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"


void UVoxelWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called for UVoxelWorldSubsystem"));

	ParseWorldGenerationSettings();
	GenerateNoiseOffset();

	const UVoxelWorldSettings* Settings = GetWorldSettings();
	GenerationRadius = Settings->GenerationRadius;
	GeneratePendingChunks(Settings->CenterChunk, GenerationRadius);
}

void UVoxelWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OnGenerationStep.Broadcast(GenerationBatchSize);
	BuildNextBatch();
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

AVoxelChunk* UVoxelWorldSubsystem::FindOrCreateChunk(FIntVector ChunkIndex)
{
	AVoxelChunk* TargetChunk = Chunks.FindRef(ChunkIndex);
	if(IsValid(TargetChunk))
	{
		return TargetChunk;
	}

	TargetChunk = SpawnNewChunk(ChunkIndex);

	return TargetChunk;
}

void UVoxelWorldSubsystem::GeneratePendingChunks(FIntPoint CenterChunk, int32 MaximumRadius)
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
					FIntVector ChunkIndex(CenterChunk.X + X, CenterChunk.Y + Y, 0);

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

void UVoxelWorldSubsystem::UnloadDepartingChunks(FIntVector PreviousCenter, FIntVector CurrentCenter, int32 KeepRadius)
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
	GenerationBatchSize = Settings->InitialGenerationBatchSize;
	InitialGenerationBatchSize = Settings->InitialGenerationBatchSize;
	GameplayGenerationBatchSize = Settings->GameplayGenerationBatchSize;
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

void UVoxelWorldSubsystem::CheckPlayerChunk()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!IsValid(PlayerCharacter)) { return; }

	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	const FIntVector BlockPosition = UWorldGenerationFunctionLibrary::BlockWorldLocationToGlobalPosition(PlayerLocation, BlockSize, BlockScale);
	const FIntVector ChunkIndex = UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(BlockPosition, ChunkSize3D);

	if (LastPlayerChunkIndex == ChunkIndex)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Player moved to a new chunk: %s"), *ChunkIndex.ToString());

	// For example, load within 6 chunks and unload beyond 8 chunks. 
	// That gap prevents chunks repeatedly spawning and despawning 
	// when the player walks back and forth along a boundary

	GeneratePendingChunks(FIntPoint(ChunkIndex.X, ChunkIndex.Y), GenerationRadius);

	UnloadDepartingChunks(LastPlayerChunkIndex, ChunkIndex, GenerationRadius + UnloadMargin);

	LastPlayerChunkIndex = ChunkIndex;
}

bool UVoxelWorldSubsystem::UnloadChunk(FIntVector ChunkIndex)
{
	const TObjectPtr<AVoxelChunk>* TargetChunkPtr = Chunks.Find(ChunkIndex);
	if (!TargetChunkPtr || !IsValid(TargetChunkPtr->Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Chunk not found for unloading: %s"), *ChunkIndex.ToString());
		return false;
	}

	AVoxelChunk* TargetChunk = TargetChunkPtr->Get();
	if (TargetChunk->IsGenerationPending())
	{
		PendingChunks.Cancel(ChunkIndex);
	}

	if (ActiveChunk == TargetChunk)
	{
		ActiveChunk = nullptr;
	}

	TargetChunk->CancelGenerationAndClear();
	return true;
}

void UVoxelWorldSubsystem::OnInitialWorldGenerated()
{
	OnInitialWorldReady.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		ChunkCheckTimerHandle,
		this,
		&UVoxelWorldSubsystem::CheckPlayerChunk,
		ChunkCheckTimerDelta,  // Interval in seconds
		true   // Repeat
	);

	GenerationBatchSize = GameplayGenerationBatchSize;
}
