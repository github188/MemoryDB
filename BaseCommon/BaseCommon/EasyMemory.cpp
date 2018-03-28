
#include "EasyMemory.h"

size_t ALIGNED_SIZE(size_t s, size_t alignedByte )
{
	size_t x = (s+(alignedByte-1)) / alignedByte;
	return x * alignedByte;
}

#if DEBUG_MODE
#include "FileDataStream.h"

#define MEM_LOG(msg, ...)		{ printf(msg, ##__VA_ARGS__); fprintf(logFile, msg, ##__VA_ARGS__); fflush(logFile);}

void EasyMemory::PrintMemory( const char *szInfo )
{
	FILE *logFile = fopen(szInfo, "at");
	MEM_LOG("\nbegin print [%s]\n-----------------------------------------------\n", szInfo);
	
	//if ((void*)mHeadBlock>mHeadPoint)
	//{
	//	for (int i = 0; i<(unsigned char*)mHeadBlock - (unsigned char*)mHeadPoint; ++i)
	//	{
	//		printf("M");
	//	}
	//}
	Block *pCurrent = mHeadBlock;
	Block *pLastBlock = pCurrent;
	size_t count = 0;
	while (pCurrent)
	{
		++count;
		for (int i = 0; i<pCurrent->mSize; ++i)
		{
			MEM_LOG("=");
		}
		if (pCurrent->mNext)
		{
			for (int i = 0; i<(unsigned char*)(pCurrent->mNext)-(unsigned char*)pCurrent->endPtr(); ++i)
			{
				MEM_LOG("M");
			}
		}
		pLastBlock = pCurrent;
		pCurrent = pCurrent->mNext;
	}
	if (pLastBlock)
	{
		for (int i = 0; i<_getEnd() -(char*)pLastBlock->endPtr(); ++i)
		{
			MEM_LOG("M");
		}
	}
	MEM_LOG("\n-----------------------------------------------\nend print total free block count [%llu]\n", count);
	fclose(logFile);
}

#endif