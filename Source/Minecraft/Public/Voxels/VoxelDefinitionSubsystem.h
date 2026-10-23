#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

	UPROPERTY(BlueprintAssignable, Category = "Definitions")
	FOnDefinitionsReady OnDefinitionsReady;

	/** Loaded resource definitions; null until loading completes. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Definitions")
	TObjectPtr<UDataTable> ResourceDefinitions = nullptr;

	/** Loaded block definitions; null until loading completes. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Definitions")
	TObjectPtr<UDataTable> BlockDefinitions = nullptr;

private:
	void HandleDefinitionsLoaded();

	TSharedPtr<FStreamableHandle> LoadHandle;
};
