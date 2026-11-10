#include "SaveSystem/VoxelWorldSaveGame.h"

UVoxelWorldSaveGame::UVoxelWorldSaveGame()
{
}

void UVoxelWorldSaveGame::SaveChunkDelta(const FIntVector ChunkIndex, const FVoxelChunkDelta& Delta)
{
	ChunkDeltas.Add(ChunkIndex, Delta);
}

bool UVoxelWorldSaveGame::ReadChunkDelta(FIntVector ChunkIndex, FVoxelChunkDelta& OutDelta)
{
	FVoxelChunkDelta* TargetChunkDelta = ChunkDeltas.Find(ChunkIndex);
	if (!TargetChunkDelta) { return false; }

	OutDelta = *TargetChunkDelta;
	return true;
}



