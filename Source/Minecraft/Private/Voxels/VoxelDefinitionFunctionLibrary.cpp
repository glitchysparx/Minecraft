#include "Voxels/VoxelDefinitionFunctionLibrary.h"
#include "Voxels/VoxelDefinitionSettings.h"


bool UVoxelDefinitionFunctionLibrary::FindResourceDefinition(/*UDataTable* Table, */EResourceType ResourceType, FResourceDefinition& OutDefinition)
{
    OutDefinition = FResourceDefinition{};

    const UVoxelDefinitionSettings* Settings = GetDefault<UVoxelDefinitionSettings>();

	// Asset is loaded by DefinitionSubsystem, so we can just get it synchronously here
    UDataTable* Table = Settings->ResourceDefinitions.Get();
    if (!Table || Table->GetRowStruct() != FResourceDefinition::StaticStruct()) { return false; }

    TArray<FResourceDefinition*> Rows;
    Table->GetAllRows<FResourceDefinition>(TEXT("FindResourceDefinition"), Rows);

    for (const FResourceDefinition* Row : Rows)
    {
        if (Row && Row->ResourceType == ResourceType)
        {
            OutDefinition = *Row;
            return true;
        }
    }

	return false;
}

bool UVoxelDefinitionFunctionLibrary::FindBlockDefinition(/*UDataTable* Table, */EVoxelBlockType BlockType, FBlockDefinition& OutDefinition)
{
	OutDefinition = FBlockDefinition{};

    const UVoxelDefinitionSettings* Settings = GetDefault<UVoxelDefinitionSettings>();

    // Asset is loaded by DefinitionSubsystem, so we can just get it synchronously here
    UDataTable* Table = Settings->BlockDefinitions.Get();
	if(!Table || Table->GetRowStruct() != FBlockDefinition::StaticStruct()) { return false; }

    TArray<FBlockDefinition*> Rows;
    Table->GetAllRows<FBlockDefinition>(TEXT("FindBlockDefinition"), Rows);

    for (const FBlockDefinition* Row : Rows)
    {
        if (Row && Row->BlockType == BlockType)
        {
            OutDefinition = *Row;
            return true;
        }
    }

	return false;
}
