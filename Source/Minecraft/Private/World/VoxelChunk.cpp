#include "World/VoxelChunk.h"

AVoxelChunk::AVoxelChunk()
{
	// The world subsystem schedules generation; chunks do not need their own Tick.
	PrimaryActorTick.bCanEverTick = false;
}

