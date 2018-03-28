//----------------------------------------------------------------------------------------
//
//  created:	9.20.2012
//	filename: 	SceneConstructionCommon.h
//  Author : 	LC
//	
//----------------------------------------------------------------------------------------
#ifndef _INCLUDE_SCENECONSTRUCTIONCOMMON_H_
#define _INCLUDE_SCENECONSTRUCTIONCOMMON_H_
//#include "Common.h"
#include "CommonDefine.h"

enum CONSTRUCTION_STATE
{
	CONSTRUCTION_STATE_FREE,
	CONSTRUCTION_STATE_BUILD,
	CONSTRUCTION_STATE_PRODUCTION,
	CONSTRUCTION_STATE_RIPE,
};

// construction income money delta
const float DELTA_CONSTRUCTION_INCOME_MONEY = 0.2f;

#endif //_INCLUDE_SCENECONSTRUCTIONCOMMON_H_