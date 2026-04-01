#include "StaticBuffer.h"
#include <cstring>

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer()
{
	for (int block = 0; block <= 3; ++block)
	{
		unsigned char buffer[BLOCK_SIZE];
		Disk::readBlock(buffer, block);
		memcpy((blockAllocMap + block * BLOCK_SIZE), buffer, BLOCK_SIZE);
	}

	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; ++bufferIndex)
	{
		metainfo[bufferIndex].free = true;
		metainfo[bufferIndex].dirty = false;
		metainfo[bufferIndex].timeStamp = -1;
		metainfo[bufferIndex].blockNum = -1;
	}
}

StaticBuffer::~StaticBuffer()
{
	for (int block = 0; block <= 3; ++block)
	{
		unsigned char buffer[BLOCK_SIZE];
		memcpy(buffer, (blockAllocMap + block * BLOCK_SIZE), BLOCK_SIZE);
		Disk::writeBlock(buffer, block);
	}

	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; ++bufferIndex)
	{
		if (metainfo[bufferIndex].free == false && metainfo[bufferIndex].dirty == true)
		{
			Disk::writeBlock(blocks[bufferIndex], metainfo[bufferIndex].blockNum);
		}
	}
}

int StaticBuffer::getStaticBlockType(int blockNum)
{
	if (blockNum < 0 || blockNum >= DISK_BLOCKS)
		return E_OUTOFBOUND;

	return (int)blockAllocMap[blockNum];
}

int StaticBuffer::setDirtyBit(int blockNum)
{
	int bufferNum = getBufferNum(blockNum);

	if (bufferNum == E_BLOCKNOTINBUFFER)
		return E_BLOCKNOTINBUFFER;

	if (bufferNum == E_OUTOFBOUND)
		return E_OUTOFBOUND;

	metainfo[bufferNum].dirty = true;

	return SUCCESS;
}

int StaticBuffer::getFreeBuffer(int blockNum)
{
	if (blockNum < 0 || blockNum >= DISK_BLOCKS)
		return E_OUTOFBOUND;

	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; ++bufferIndex)
	{
		if (metainfo[bufferIndex].free == false)
		{
			metainfo[bufferIndex].timeStamp++;
		}
	}

	int bufferNum = -1;

	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; ++bufferIndex)
	{
		if (metainfo[bufferIndex].free == true)
		{
			bufferNum = bufferIndex;
			break;
		}
	}

	if (bufferNum == -1)
	{
		int maxtimestamp = 0;
		for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; ++bufferIndex)
		{
			if (metainfo[bufferIndex].timeStamp > maxtimestamp)
			{
				maxtimestamp = metainfo[bufferIndex].timeStamp;
				bufferNum = bufferIndex;
			}
		}
		if (metainfo[bufferNum].dirty == true)
		{
			Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
		}
	}

	metainfo[bufferNum].free = false;
	metainfo[bufferNum].dirty = false;
	metainfo[bufferNum].blockNum = blockNum;
	metainfo[bufferNum].timeStamp = 0;

	return bufferNum;
}

int StaticBuffer::getBufferNum(int blockNum)
{
	if (blockNum < 0 || blockNum >= DISK_BLOCKS)
	{
		return E_OUTOFBOUND;
	}

	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; ++bufferIndex)
	{
		if (metainfo[bufferIndex].blockNum == blockNum)
		{
			return bufferIndex;
		}
	}

	return E_BLOCKNOTINBUFFER;
}
