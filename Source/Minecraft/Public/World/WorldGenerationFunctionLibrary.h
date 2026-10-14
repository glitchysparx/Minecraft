

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WorldGenerationFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MINECRAFT_API UWorldGenerationFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// SamplePerlinNoise2D samples Perlin noise at a given grid coordinate, frequency, and noise offset.
	// @returns a value between -1.0 and 1.0
	UFUNCTION(BlueprintPure, Category = "WorldGeneration|Math")
	static float SamplePerlinNoise2D (	int32 GridX, int32 GridY, float Frequency,
										const FVector2D& NoiseOffset );
	
	// CalculateTerrainHeight calculates the terrain height at a given grid coordinate based on Perlin noise.
	// @returns an integer height value between MinHeight and MaxHeight
	UFUNCTION(BlueprintPure, Category = "WorldGeneration|Math")
	static int32 CalculateTerrainHeight (	int32 GridX, int32 GridY, float Frequency,
											const FVector2D& NoiseOffset, int32 MinHeight,
											int32 MaxHeight);
};
