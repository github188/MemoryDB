//----------------------------------------------------------------------------------------
//
//  created:	9.20.2012
//	filename: 	SceneFieldCommon.h
//  Author : 	LC
//	
//----------------------------------------------------------------------------------------
#ifndef _INCLUDE_SCENEFIELDCOMMON_H_
#define _INCLUDE_SCENEFIELDCOMMON_H_
//#include "Common.h"
#include "CommonDefine.h"

enum GROW_STATE
{
	GROW_NULL = -1,
	GROW_STATE_FREE = 0,
	GROW_STATE_GROW,
	GROW_STATE_RIPE,
	GROW_STATE_WITHER,
	GROW_STATE_RELIVE,
};

// field income money delta
const float DELTA_FIELD_INCOME_MONEY = 0.2f;
const float DELTA_FIELD_INCOME_EXP = 0.2f;

#endif //_INCLUDE_SCENEFIELDCOMMON_H_