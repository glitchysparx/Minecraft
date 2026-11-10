#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystem/VoxelChunkDelta.h"
#include "World/VoxelWorldGenerationSettings.h"
#include "VoxelWorldSaveGame.generated.h"


UCLASS()
class MINECRAFT_API UVoxelWorldSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:	
	UVoxelWorldSaveGame();

	// Save format version for future migrations.
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Save")
	int32 SaveVersion = 1;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Save")
	FVoxelWorldGenerationSettings GenerationSettings;

	void SaveChunkDelta(const FIntVector ChunkIndex, const FVoxelChunkDelta& Delta);
	bool ReadChunkDelta(const FIntVector ChunkIndex, FVoxelChunkDelta& OutDelta);

protected:
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Save")
	TMap<FIntVector, FVoxelChunkDelta> ChunkDeltas;
};
