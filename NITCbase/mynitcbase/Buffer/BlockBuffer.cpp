#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

BlockBuffer::BlockBuffer(int blockNum)
{
	this->blockNum = blockNum;
}

BlockBuffer::BlockBuffer(char blockType)
{
	int blockTypeEnum;
	switch (blockType)
	{
	case 'R':
		blockTypeEnum = REC;
		break;
	case 'I':
		blockTypeEnum = IND_INTERNAL;
		break;
	case 'L':
		blockTypeEnum = IND_LEAF;
		break;
	default:
		break;
	}
	int blockNum = getFreeBlock(blockTypeEnum);
	this->blockNum = blockNum;
}

int BlockBuffer::getBlockNum()
{
	return this->blockNum;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{
	int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

	if (bufferNum != E_BLOCKNOTINBUFFER)
	{
		for (int i = 0; i < BUFFER_CAPACITY; i++)
		{
			if (StaticBuffer::metainfo[i].free == false)
				StaticBuffer::metainfo[i].timeStamp++;
		}
		StaticBuffer::metainfo[bufferNum].timeStamp = 0;
	}
	else
	{
		bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

		if (bufferNum == E_OUTOFBOUND)
			return E_OUTOFBOUND;

		Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
	}

	*buffPtr = StaticBuffer::blocks[bufferNum];

	return SUCCESS;
}

int BlockBuffer::getHeader(struct HeadInfo *head)
{
	unsigned char *bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS)
		return ret;

	memcpy(&head->numSlots, bufferPtr + 24, 4);
	memcpy(&head->numAttrs, bufferPtr + 20, 4);
	memcpy(&head->numEntries, bufferPtr + 16, 4);
	memcpy(&head->rblock, bufferPtr + 12, 4);
	memcpy(&head->lblock, bufferPtr + 8, 4);
	memcpy(&head->pblock, bufferPtr + 4, 4);
	memcpy(&head->blockType, bufferPtr, 4);

	return SUCCESS;
}

int BlockBuffer::setHeader(struct HeadInfo *head)
{
	unsigned char *bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS)
		return ret;

	struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

	bufferHeader->blockType = head->blockType;
	bufferHeader->pblock = head->pblock;
	bufferHeader->lblock = head->lblock;
	bufferHeader->rblock = head->rblock;
	bufferHeader->numEntries = head->numEntries;
	bufferHeader->numAttrs = head->numAttrs;
	bufferHeader->numSlots = head->numSlots;

	ret = StaticBuffer::setDirtyBit(this->blockNum);
	if (ret != SUCCESS)
		return ret;

	return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType)
{
	unsigned char *bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS)
		return ret;

	*((int32_t *)bufferPtr) = blockType;

	StaticBuffer::blockAllocMap[this->blockNum] = blockType;

	ret = StaticBuffer::setDirtyBit(this->blockNum);
	if (ret != SUCCESS)
		return ret;

	return SUCCESS;
}

int BlockBuffer::getFreeBlock(int blockType)
{
	int freeBlock = -1;
	for (int block = 0; block < BLOCK_SIZE; block++)
	{
		if (StaticBuffer::blockAllocMap[block] == UNUSED_BLK)
		{
			freeBlock = block;
			break;
		}
	}

	if (freeBlock == -1)
		return E_DISKFULL;

	this->blockNum = freeBlock;

	int bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

	HeadInfo head{.pblock = -1,
				  .lblock = -1,
				  .rblock = -1,
				  .numEntries = 0,
				  .numAttrs = 0,
				  .numSlots = 0};
	setHeader(&head);

	setBlockType(blockType);

	return freeBlock;
}

void BlockBuffer::releaseBlock()
{
	if (this->blockNum == INVALID_BLOCKNUM)
		return;
	else
	{
		int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
		if (bufferNum != E_BLOCKNOTINBUFFER)
		{
			StaticBuffer::metainfo[bufferNum].free = true;
		}
		StaticBuffer::blockAllocMap[this->blockNum] = UNUSED_BLK;
		this->blockNum = INVALID_BLOCKNUM;
	}
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

RecBuffer::RecBuffer() : BlockBuffer('R') {}

int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
	struct HeadInfo head;

	this->getHeader(&head);

	int attrCount = head.numAttrs;
	int slotCount = head.numSlots;

	unsigned char *bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS)
	{
		return ret;
	}

	int recordSize = attrCount * ATTR_SIZE;
	unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

	memcpy(rec, slotPointer, recordSize);

	return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum)
{
	unsigned char *bufferPtr;
	int res = loadBlockAndGetBufferPtr(&bufferPtr);

	if (res != SUCCESS)
		return res;

	struct HeadInfo head;
	this->getHeader(&head);

	int attrCount = head.numAttrs;
	int slotCount = head.numSlots;

	if (slotNum < 0 || slotNum >= slotCount)
		return E_OUTOFBOUND;

	int recordSize = attrCount * ATTR_SIZE;
	unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

	memcpy(slotPointer, rec, recordSize);

	StaticBuffer::setDirtyBit(this->blockNum);

	return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap)
{
	unsigned char *bufferPtr;

	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS)
	{
		return ret;
	}

	struct HeadInfo head;
	this->getHeader(&head);
	int slotCount = head.numSlots;

	unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

	memcpy(slotMap, slotMapInBuffer, slotCount);
	return SUCCESS;
}

int RecBuffer::setSlotMap(unsigned char *slotMap)
{
	unsigned char *bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS)
	{
		return ret;
	}

	struct HeadInfo head;
	this->getHeader(&head);
	int numSlots = head.numSlots;

	unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

	memcpy(slotMapInBuffer, slotMap, numSlots);

	ret = StaticBuffer::setDirtyBit(this->blockNum);
	if (ret != SUCCESS)
		return ret;

	return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType)
{

	double diff;
	if (attrType == STRING)
		diff = strcmp(attr1.sVal, attr2.sVal);

	else
		diff = attr1.nVal - attr2.nVal;

	if (diff > 0)
		return 1;
	if (diff < 0)
		return -1;
	if (diff == 0)
		return 0;
}
