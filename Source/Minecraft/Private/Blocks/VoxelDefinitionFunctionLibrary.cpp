#include "Blocks/VoxelDefinitionFunctionLibrary.h"
#include "Blocks/VoxelDefinitionSettings.h"

bool UVoxelDefinitionFunctionLibrary::FindResourceDefinition(/*UDataTable* Table, */EResourceType ResourceType, FResourceDefinition& OutDefinition)
{
    OutDefinition = FResourceDefinition{};

    // temporary bad solution ----------------------------------- (it should be loaded once on the game start step asynchronously)
    const UVoxelDefinitionSettings* Settings =
        GetDefault<UVoxelDefinitionSettings>();

    UDataTable* Table =
        Settings->ResourceDefinitions.LoadSynchronous();
    // ----------------------------------------------------------

    if (!Table || Table->GetRowStruct() != FResourceDefinition::StaticStruct()) { return false; }

    TArray<FResourceDefinition*> Rows;
    Table->GetAllRows<FResourceDefinition>(
        TEXT("FindResourceDefinition"), Rows);

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

    // temporary bad solution -----------------------------------
    const UVoxelDefinitionSettings* Settings =
        GetDefault<UVoxelDefinitionSettings>();

    UDataTable* Table =
        Settings->BlockDefinitions.LoadSynchronous();
    // ----------------------------------------------------------

	if(!Table || Table->GetRowStruct() != FBlockDefinition::StaticStruct()) { return false; }

    TArray<FBlockDefinition*> Rows;
    Table->GetAllRows<FBlockDefinition>(
        TEXT("FindBlockDefinition"), Rows);

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
