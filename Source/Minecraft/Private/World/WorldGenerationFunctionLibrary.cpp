#include "World/WorldGenerationFunctionLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"


float UWorldGenerationFunctionLibrary::SamplePerlinNoise2D(int32 GridX, int32 GridY, float Frequency, const FVector2D& NoiseOffset)
{
	// NoiseOffset is chosen once for the whole generated world.
	const FVector2D SamplePosition = FVector2D(GridX, GridY) * Frequency + NoiseOffset;

	const float Noise = FMath::PerlinNoise2D(SamplePosition); // Sample Perlin noise at the calculated position
	return Noise;
}

int32 UWorldGenerationFunctionLibrary::CalculateTerrainHeight(int32 GridX, int32 GridY, float Frequency, const FVector2D& NoiseOffset, int32 MinHeight, int32 MaxHeight)
{
	const float Noise = SamplePerlinNoise2D(GridX, GridY, Frequency, NoiseOffset);

	const float NormalizedNoise =
		FMath::Clamp((Noise + 1.0f) * 0.5f, 0.0f, 1.0f); // Normalize noise from [-1, 1] to [0, 1]

	const float Height = FMath::Lerp(float(MinHeight), float(MaxHeight), NormalizedNoise); // Interpolate between MinHeight and MaxHeight based on normalized noise

	const int32 FinalHeight = FMath::RoundToInt(Height);
	return FinalHeight;
}

FIntVector UWorldGenerationFunctionLibrary::BlockLocalPositionToGlobal(FIntVector LocalPosition, FIntVector ChunkSize, FIntVector ChunkIndex)
{
	const FIntVector GlobalPosition = LocalPosition + ChunkIndex * ChunkSize;
	return GlobalPosition;
}

FIntVector UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocal(FIntVector GlobalPosition, FIntVector ChunkSize)
{
	const FIntVector ChunkIndex = BlockGlobalPositionToChunkIndex(GlobalPosition, ChunkSize);
	return BlockGlobalPositionToLocalByChunkIndex(GlobalPosition, ChunkSize, ChunkIndex);
}

FIntVector UWorldGenerationFunctionLibrary::BlockGlobalPositionToLocalByChunkIndex(FIntVector GlobalPosition, FIntVector ChunkSize, FIntVector ChunkIndex)
{
	const FIntVector LocalPosition = GlobalPosition - ChunkIndex * ChunkSize;
	return LocalPosition;
}

FIntVector UWorldGenerationFunctionLibrary::BlockGlobalPositionToChunkIndex(FIntVector GlobalPosition, FIntVector ChunkSize)
{
	const FIntVector ChunkIndex(
		FMath::FloorToInt(
			static_cast<double>(GlobalPosition.X) / ChunkSize.X),
		FMath::FloorToInt(
			static_cast<double>(GlobalPosition.Y) / ChunkSize.Y),
		FMath::FloorToInt(
			static_cast<double>(GlobalPosition.Z) / ChunkSize.Z)
	);

	return ChunkIndex;
}

FIntVector UWorldGenerationFunctionLibrary::BlockIndexToLocalGridPosition(int32 Index, FIntVector ChunkSize)
{
	FIntVector LocalPosition = FIntVector();
	LocalPosition.X = Index % ChunkSize.X;
	LocalPosition.Y = Index / ChunkSize.X % ChunkSize.Y;
	LocalPosition.Z = Index / (ChunkSize.X * ChunkSize.Y);

	return LocalPosition;
}

int32 UWorldGenerationFunctionLibrary::BlockLocalGridPositionToIndex(FIntVector LocalPosition, FIntVector ChunkSize)
{
	const int32 Index = LocalPosition.X + LocalPosition.Y * ChunkSize.X + LocalPosition.Z * ChunkSize.X * ChunkSize.Y;
	return Index;
}

FIntVector UWorldGenerationFunctionLibrary::BlockWorldLocationToGlobalPosition(FVector WorldLocation, FVector BlockSize, FVector BlockScale)
{
	const FVector CellSize = BlockSize * BlockScale;
	const FVector GridPosition = WorldLocation / CellSize;

	return FIntVector(
		FMath::FloorToInt(GridPosition.X),
		FMath::FloorToInt(GridPosition.Y),
		FMath::FloorToInt(GridPosition.Z)
	);
}

FVector UWorldGenerationFunctionLibrary::BlockLocalPositionToChunkLocalLocation(FIntVector LocalPosition, FVector BlockSize, FVector BlockScale)
{
	return FVector(LocalPosition)* BlockSize* BlockScale;
}

int32 UWorldGenerationFunctionLibrary::FindBlockInstanceIndexByLocation(const UInstancedStaticMeshComponent* Component, FVector TargetLocation, bool bWorldSpace)
{
	if (!IsValid(Component)) { return INDEX_NONE; }

	constexpr double LocationTolerance = 0.1;
	
	const int32 InstanceCount = Component->GetInstanceCount();

	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		FTransform Transform;

		if (!Component->GetInstanceTransform(Index, Transform, bWorldSpace)) { continue; }

		if (Transform.GetLocation().Equals(TargetLocation, LocationTolerance))
		{
			return Index;
		}
	}

	return INDEX_NONE;
}
