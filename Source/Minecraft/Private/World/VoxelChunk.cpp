#include "World/VoxelChunk.h"
#include "SaveSystem/VoxelChunkDelta.h"
#include <World/WorldGenerationFunctionLibrary.h>

AVoxelChunk::AVoxelChunk()
{
	// The world subsystem schedules generation; chunks do not need their own Tick.
	PrimaryActorTick.bCanEverTick = false;
}


bool AVoxelChunk::MarkGenerationPending()
{
    if (GenerationState != EChunkGenerationState::Empty)
    {
        return false;
    }

    SetGenerationState(EChunkGenerationState::Pending);
    return true;
}


FVoxelChunkDelta AVoxelChunk::GetChunkModification() const 
{
    FVoxelChunkDelta Delta;
    for (int32 BlockIndex : ModifiedBlocks)
    {
        if (Blocks.IsValidIndex(BlockIndex))
        {
            FVoxelBlockModification Modification;
            Modification.BlockIndex = BlockIndex;
            Modification.BlockType = Blocks[BlockIndex].bHasInstance ? Blocks[BlockIndex].Type : EVoxelBlockType::Air;
            Delta.Modifications.Add(Modification);
        }
    }
	return Delta;
}

void AVoxelChunk::MergeChunkModification(const FVoxelChunkDelta& Delta)
{
    TMap<int32, EVoxelBlockType> RemainingModifications;
    RemainingModifications.Reserve(Delta.Modifications.Num());

    // 1. Create lookup map.
    for (const FVoxelBlockModification& Modification : Delta.Modifications)
    {
        RemainingModifications.Add(
            Modification.BlockIndex,
            Modification.BlockType);
    }

    // 2. Process existing generated blocks.
    for (FPendingBlockInstance& Block : PendingBlocks)
    {
        if (const EVoxelBlockType* ModifiedType =
            RemainingModifications.Find(Block.BlockIndex))
        {
            Block.Type = *ModifiedType;
            RemainingModifications.Remove(Block.BlockIndex);
        }
    }

    // 3. Process blocks not found in generated terrain.
    for (const auto& [Index, Type] : RemainingModifications)
    {
        if (Type == EVoxelBlockType::Air)
        {
            continue;
        }

        FPendingBlockInstance NewBlock;
        NewBlock.BlockIndex = Index;
        NewBlock.Type = Type;

        const FIntVector LocalGridPosition = UWorldGenerationFunctionLibrary::BlockIndexToLocalGridPosition(Index, GetChunkSize3D());
        NewBlock.Transform = MakeBlockLocalTransform(LocalGridPosition);

        PendingBlocks.Add(MoveTemp(NewBlock));
    }

}

FTransform AVoxelChunk::MakeBlockLocalTransform(const FIntVector& LocalGridPosition) const
{
    FTransform Result = FTransform();
    Result.SetLocation(
        UWorldGenerationFunctionLibrary::BlockLocalPositionToChunkLocalLocation(
            LocalGridPosition, BlockSize, BlockScale));
    Result.SetScale3D(BlockScale);

    return Result;
}

