#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "World/VoxelBlock.h"
#include "VoxelDefinitionSubsystem.generated.h"

class UDataTable;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDefinitionsReady);

/** Owns shared block and resource definitions for the game instance. */
UCLASS()
class MINECRAFT_API UVoxelDefinitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// The definitions are shared by players, NPCs, and UI. They don’t belong to a particular player. 
	// Its lifetime also allows the loaded tables to remain available across map changes.

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Definitions")
	TArray<EVoxelBlockType> GetBuildingBlockTypeOrder() const
	{
		return BuildingBlockTypeOrder;
	}

	UPROPERTY(BlueprintAssignable, Category = "Definitions")
	FOnDefinitionsReady OnDefinitionsReady;

	/** Loaded resource definitions; null until loading completes. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Definitions")
	TObjectPtr<UDataTable> ResourceDefinitions = nullptr;

	/** Loaded block definitions; null until loading completes. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Definitions")
	TObjectPtr<UDataTable> BlockDefinitions = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Definitions")
	bool AreDefinitionsReady() const
	{
		return ResourceDefinitions != nullptr && BlockDefinitions != nullptr;
	}

private:
	void HandleDefinitionsLoaded();

	UFUNCTION()
	void InitializeBuildingBlockOrder();

	TSharedPtr<FStreamableHandle> LoadHandle;

	UPROPERTY(Transient)
	TArray<EVoxelBlockType> BuildingBlockTypeOrder;
};
