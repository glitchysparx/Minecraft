#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"


class MINECRAFT_API FPendingChunkQueue
{
public:
	/** Returns false if this coordinate is already pending. */
	bool Enqueue(const FIntVector& ChunkIndex);

	/** Cancels a pending request. Its stale queue entry is skipped during dequeue. */
	bool Cancel(const FIntVector& ChunkIndex);

	/** Removes the next active request; leaves OutChunkIndex unchanged if empty. */
	bool Dequeue(FIntVector& OutChunkIndex);

	bool Contains(const FIntVector& ChunkIndex) const;
	void Reset();
	int32 Num() const { return PendingRequests.Num(); }

private:
	struct FRequest
	{
		FIntVector ChunkIndex;
		uint64 RequestId;
	};

	TQueue<FRequest> Queue; // Keep track of the order of requests
	TMap<FIntVector, uint64> PendingRequests; // Keep track of the latest request ID for each chunk index
	uint64 NextRequestId = 0;
};
