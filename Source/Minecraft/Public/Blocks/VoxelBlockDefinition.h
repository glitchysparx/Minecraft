#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Inventory/VoxelResources.h"
#include "World/VoxelBlock.h"
#include "VoxelBlockDefinition.generated.h"

class UTexture2D;

/** Static block information, including placement costs and destruction drops. */
USTRUCT(BlueprintType)
struct MINECRAFT_API FBlockDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	EVoxelBlockType BlockType = EVoxelBlockType::Air;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	EResourceType RequiredResource = EResourceType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0", UIMin = "0"))
	int32 RequiredAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
	EResourceType DroppedResource = EResourceType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops", meta = (ClampMin = "0", UIMin = "0"))
	int32 DropAmount = 0;
};
