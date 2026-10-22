#pragma once

#include "CoreMinimal.h"
#include "VoxelBlock.generated.h"

/** Logical block types for voxel world */
UENUM(BlueprintType)
enum class EVoxelBlockType : uint8
{
	Air		UMETA(DisplayName = "Air"),
	Rock	UMETA(DisplayName = "Rock"),
	Grass	UMETA(DisplayName = "Grass"),
	Snow	UMETA(DisplayName = "Snow")
};


USTRUCT(BlueprintType)
struct FVoxelBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVoxelBlockType Type = EVoxelBlockType::Air;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasInstance = false;
};
