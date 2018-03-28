#include "RandTool.h"
#ifndef _CRT_RAND_S
#	define _CRT_RAND_S
#endif
#include <stdlib.h>


//void CRand::SetSpeed(uint32_t u, uint32_t v)
//{
//	mSimple.SetSeed(u, v);
//	srand(u);
//}
//

void CRand::SetSeed(uint32_t seed)
{
    srand(seed);
}

uint CRand::Rand(uint maxInt)
{
    //maxInt = maxInt&0xFFFF;

    //if (maxInt<=RAND_MAX)
    //    return rand()%(maxInt+1);

    //return ((rand()%(RAND_MAX+1)) + (rand()%(RAND_MAX+1)))%(maxInt+1);

	uint x = 0;
	rand_s(&x);
	return x%(maxInt+1);

	//if (maxInt<=RAND_MAX)
	//{
	//	return (uint)((double)rand()/RAND_MAX * maxInt+0.5f);
	//}

	//if (maxInt<=0xFFFF)
	//{
	//	uint x = (rand()%256) + ((rand()%256)<<8);

	//	return (uint) ((double)x/0xFFFF*maxInt+0.5f);
	//}
	//if (maxInt<0xFFFFFF)
	//{
	//	uint x = (rand()%256) + ((rand()%256)<<8) + ((rand()%256)<<16);
	//	return (uint) ((double)x/0xFFFFFF*maxInt+0.5f);
	//}
	//uint x = (rand()%256) + ((rand()%256)<<8) + ((rand()%256)<<16) + ((rand()%256)<<24);
	//return (uint) ((double)x/0xFFFFFFFF*maxInt+0.5f);
}

//CRand::SimpleRNG CRand::mSimple;