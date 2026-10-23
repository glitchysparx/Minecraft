#include "Voxels/VoxelDefinitionSubsystem.h"
#include "Voxels/VoxelDefinitionSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UVoxelDefinitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    const UVoxelDefinitionSettings* Settings = GetDefault<UVoxelDefinitionSettings>();

    if (Settings->ResourceDefinitions.IsNull() || Settings->BlockDefinitions.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("Voxel definition tables are not configured."));
        return;
    }

    TArray<FSoftObjectPath> Paths;
    Paths.AddUnique(Settings->ResourceDefinitions.ToSoftObjectPath());
    Paths.AddUnique(Settings->BlockDefinitions.ToSoftObjectPath());

    LoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths, 
        FStreamableDelegate::CreateUObject(
            this, &UVoxelDefinitionSubsystem::HandleDefinitionsLoaded));
}

void UVoxelDefinitionSubsystem::Deinitialize()
{
    // when this subsystem stops, its pending loading request should stop too
    if (LoadHandle.IsValid())
    {
        LoadHandle->CancelHandle();
        LoadHandle.Reset();
    }
}

void UVoxelDefinitionSubsystem::HandleDefinitionsLoaded()
{
    const UVoxelDefinitionSettings* Settings = GetDefault<UVoxelDefinitionSettings>();

    // Get() retrieves an already loaded object
    ResourceDefinitions = Settings->ResourceDefinitions.Get();
    BlockDefinitions = Settings->BlockDefinitions.Get();

    if (ResourceDefinitions == nullptr || BlockDefinitions == nullptr) 
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load voxel definition tables."));
        return;
    }

    OnDefinitionsReady.Broadcast();
}
