#pragma once

#include "CoreMinimal.h"
#include "World/VoxelBlock.h"
#include "VoxelChunkDelta.generated.h"

USTRUCT(BlueprintType)
struct FVoxelBlockModification
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BlockIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVoxelBlockType BlockType = EVoxelBlockType::Air;   
};


USTRUCT(BlueprintType)
struct FVoxelChunkDelta
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVoxelBlockModification> Modifications;
};