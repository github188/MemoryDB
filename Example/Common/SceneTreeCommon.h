//----------------------------------------------------------------------------------------
//
//  created:	9.22.2012
//	filename: 	SceneTreeCommon.h
//  Author : 	LC
//	
//----------------------------------------------------------------------------------------
#ifndef _INCLUDE_SCENETREECOMMON_H_
#define _INCLUDE_SCENETREECOMMON_H_
//#include "Common.h"
#include "CommonDefine.h"

enum TREE_STATE
{
	TREE_STATE_FREE,
	TREE_STATE_PRODUCTION,
	TREE_STATE_RIPE,
};

// tree income money delta
const float DELTA_TREE_INCOME_MONEY = 0.2f;
const float DELTA_TREE_INCOME_EXP = 0.2f;

#endif //_INCLUDE_SCENETREECOMMON_H_