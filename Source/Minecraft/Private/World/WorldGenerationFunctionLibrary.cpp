#include "World/WorldGenerationFunctionLibrary.h"


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
