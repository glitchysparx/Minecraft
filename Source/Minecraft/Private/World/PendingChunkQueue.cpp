#include "World/PendingChunkQueue.h"

bool FPendingChunkQueue::Enqueue(const FIntVector& ChunkIndex)
{
	if (PendingRequests.Contains(ChunkIndex))
	{
		return false;
	}

	const uint64 RequestId = ++NextRequestId;
	if (!Queue.Enqueue(FRequest{ChunkIndex, RequestId}))
	{
		return false;
	}

	PendingRequests.Add(ChunkIndex, RequestId);
	return true;
}

bool FPendingChunkQueue::Cancel(const FIntVector& ChunkIndex)
{
	return PendingRequests.Remove(ChunkIndex) > 0;
}

bool FPendingChunkQueue::Dequeue(FIntVector& OutChunkIndex)
{
	FRequest Request;
	while (Queue.Dequeue(Request))
	{
		const uint64* CurrentId = PendingRequests.Find(Request.ChunkIndex);
		if (!CurrentId || *CurrentId != Request.RequestId)
		{
			continue;
		}

		PendingRequests.Remove(Request.ChunkIndex);
		OutChunkIndex = Request.ChunkIndex;
		return true;
	}

	return false;
}

bool FPendingChunkQueue::Contains(const FIntVector& ChunkIndex) const
{
	return PendingRequests.Contains(ChunkIndex);
}

void FPendingChunkQueue::Reset()
{
	Queue.Empty();
	PendingRequests.Reset();
	NextRequestId = 0;
}
