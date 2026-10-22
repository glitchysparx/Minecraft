#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "VoxelDefinitionSettings.generated.h"

/** Project-wide tables used to look up resource and block definitions. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Voxel Definitions"))
class MINECRAFT_API UVoxelDefinitionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Definitions", meta = (RequiredAssetDataTags = "RowStructure=/Script/Minecraft.ResourceDefinition"))
	TSoftObjectPtr<UDataTable> ResourceDefinitions;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Definitions", meta = (RequiredAssetDataTags = "RowStructure=/Script/Minecraft.BlockDefinition"))
	TSoftObjectPtr<UDataTable> BlockDefinitions;
};
