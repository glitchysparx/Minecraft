#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Voxels/VoxelBlockDefinition.h"
#include "Voxels/VoxelResources.h"
#include "VoxelDefinitionFunctionLibrary.generated.h"

class UDataTable;

/** Shared lookups for block and resource definitions. */
UCLASS()
class MINECRAFT_API UVoxelDefinitionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true when a definition matching ResourceType is found. */
	UFUNCTION(BlueprintCallable, Category = "Definitions")
	static bool FindResourceDefinition(/*UDataTable* Table, */EResourceType ResourceType, FResourceDefinition& OutDefinition);

	/** Returns true when a definition matching BlockType is found. */
	UFUNCTION(BlueprintCallable, Category = "Definitions")
	static bool FindBlockDefinition(/*UDataTable* Table, */EVoxelBlockType BlockType, FBlockDefinition& OutDefinition);
};
