#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerationSettings.generated.h"

USTRUCT(BlueprintType)
struct FVoxelWorldGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	FIntPoint ChunkSize2D = FIntPoint(16, 16);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.001"))
	FVector BlockSize = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.001"))
	FVector BlockScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D NoiseOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.000001"))
	float NoiseFrequency = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint MapHeightRange = FIntPoint(1, 32);

	FIntVector GetChunkSize3D() const
	{
		return FIntVector(ChunkSize2D.X, ChunkSize2D.Y, MapHeightRange.Y);
	}
};
