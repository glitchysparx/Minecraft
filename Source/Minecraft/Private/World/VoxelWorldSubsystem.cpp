#include "World/VoxelWorldSubsystem.h"
#include "World/VoxelChunk.h"
#include "World/VoxelWorldSettings.h"


void UVoxelWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called for UVoxelWorldSubsystem"));

	ParseWorldGenerationSettings();

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
					PendingChunks.Add(CenterChunk + FIntPoint(X, Y));
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
			ActiveChunk = nullptr;
		}
	}
	else
	{
		SpawnNextChunk();
	}
}

AVoxelChunk* UVoxelWorldSubsystem::SpawnNewChunk(FIntPoint TargetChunkIndex)
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

	NewChunk->InitializeChunk(TargetChunkIndex, ChunkSize, MapHeightRange, NoiseOffset, NoiseFrequency, BlockScale);

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
	}

	return ActiveChunk;
}

FVector UVoxelWorldSubsystem::CalculateChunkLocation(FIntPoint ChunkIndex)
{
	FIntPoint BlockCountFromZero = ChunkIndex * ChunkSize;
	FVector BlockFinalSize = BlockSize * BlockScale;
	return FVector(BlockCountFromZero.X * BlockFinalSize.X, BlockCountFromZero.Y * BlockFinalSize.Y, 0 );
}

const UVoxelWorldSettings* UVoxelWorldSubsystem::GetWorldSettings()
{
	return GetDefault<UVoxelWorldSettings>();
}

void UVoxelWorldSubsystem::ParseWorldGenerationSettings()
{
	const UVoxelWorldSettings* Settings = GetWorldSettings();

	WorldSize = Settings->WorldSize;
	ChunkSize = Settings->ChunkSize;
	BlockSize = Settings->BlockSize;
	BlockScale = Settings->BlockScale;
	NoiseFrequency = Settings->NoiseFrequency;
	NoiseOffset = Settings->NoiseOffset;
	MapHeightRange = Settings->MapHeightRange;
	GenerationBatchSize = Settings->GenerationBatchSize;

	// ChunkClass is a soft reference in settings.
	ChunkClass = Settings->ChunkClass.LoadSynchronous();
}
