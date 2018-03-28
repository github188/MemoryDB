#ifndef _INCLUDE_SCRIPTAI_H_
#define _INCLUDE_SCRIPTAI_H_

#include "AI.h"

extern "C" {  
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <tolua++.h>
}

#include "lua_call.hpp"
#include "LuaScriptManager.h"
#include "ScriptEvent.h"

namespace AI
{
	//-------------------------------------------------------------------------
	// AINode ½Å±¾ÊÂ¼þ
	//-------------------------------------------------------------------------
	class ScriptNodeFactory : public ScriptEventFactory
	{
	public:
		ScriptNodeFactory(lua_State *l, int stack)
			: ScriptEventFactory(l, stack)
			, mStartFun(NULL)
			, mThinkFun(NULL)
			, mOnDeadFun(NULL)
			, mOnHitFun(NULL)
			, mOnHpChangedFun(NULL)
			, mPauseFun(NULL)
			, mContinueFun(NULL)
			, mNeedStartFun(NULL)
		{
			if (stack<=0)
				return;
			ReadyFunction(stack, "start", &mStartFun, true);
			ReadyFunction(stack, "think", &mThinkFun, false);
			ReadyFunction(stack, "needStart", &mNeedStartFun, true);
			ReadyFunction(stack, "OnDead", &mOnDeadFun, false);
			ReadyFunction(stack, "OnHit", &mOnHitFun, false);
			ReadyFunction(stack, "OnHpChanged", &mOnHpChangedFun, false);
			ReadyFunction(stack, "Pause", &mPauseFun, false);
			ReadyFunction(stack, "Continue", &mContinueFun, false);
		}
		~ScriptNodeFactory()
		{
			SAFE_DELETE(mStartFun);
			SAFE_DELETE(mThinkFun);
			SAFE_DELETE(mOnDeadFun);
			SAFE_DELETE(mOnHitFun);
			SAFE_DELETE(mOnHpChangedFun);
			SAFE_DELETE(mPauseFun);
			SAFE_DELETE(mContinueFun);
			SAFE_DELETE(mNeedStartFun);
		}

	public:
		lua_function<void>	*mStartFun;
		lua_function<void>	*mThinkFun;
		lua_function<bool>	*mNeedStartFun;
		lua_function<void>  *mOnDeadFun;
		lua_function<void>  *mOnHitFun;
		lua_function<void>  *mOnHpChangedFun;

		lua_function<void>  *mPauseFun;
		lua_function<void>  *mContinueFun;
	};
	//-------------------------------------------------------------------------*/
	template<typename T>
	class DefineAINodeFactory : public ScriptNodeFactory
	{
	public:
		DefineAINodeFactory(lua_State *lua)
			: ScriptNodeFactory(lua, 0){}

	public:
		virtual AutoEvent NewEvent()
		{
			return MEM_NEW T();
		}

		virtual bool IsScript() const override { return false; }
	};
	//-------------------------------------------------------------------------
	class ScriptNode : public tNode
	{
	public:
		ScriptNode()
		{

		}
		~ScriptNode()
		{
			ClearWaitEvent();
		}

		//virtual void OnLockEnemy(HandActive enemy) override;

	public:
		virtual void start()
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mStartFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					(*GetScriptFactory()->mStartFun)(evt);
				}
				LUA_TRY_CATCH

		}
		virtual void think()
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mThinkFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					(*GetScriptFactory()->mThinkFun)(evt);
				}
				else
					tNode::think();
				LUA_TRY_CATCH
		}

		virtual bool needStart() override
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mNeedStartFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					return (*GetScriptFactory()->mNeedStartFun)(evt);
				}
				LUA_TRY_CATCH
		}

		virtual void OnDead() override
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mOnDeadFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					(*GetScriptFactory()->mOnDeadFun)(evt);
				}
				LUA_TRY_CATCH
		}

		virtual void OnHit(int attacker) override
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mOnHitFun)
				{	
					LuaEvent *evt = GET_LUA( GetSelf());
					LuaActive *attackerObj = NULL;					
					HandActive owner = GetOwner();
					
					HandActive att;

					if (owner && owner->GetScene()!=NULL)
						att = owner->GetScene()->FindActive(attacker);

					if (att)
						attackerObj = GET_LUA_ACTIVE(att);

					//LuaActive attackerObj;
					//attackerObj.mActive = attacker;

					(*GetScriptFactory()->mOnHitFun)(evt, attackerObj);
				}

				LUA_TRY_CATCH
		}

		virtual void OnHpChanged(int oldHp) override
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mOnHpChangedFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());					

					(*GetScriptFactory()->mOnHpChangedFun)(evt, oldHp);
				}
				LUA_TRY_CATCH
		}

	public:
		virtual bool _OnTimeOver()
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mTimeOverFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					(*GetScriptFactory()->mTimeOverFun)(evt);
				}
				LUA_TRY_CATCH
					return true;
		}


		virtual bool Update(float t)
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mUpdateFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					return (*GetScriptFactory()->mUpdateFun)(evt);
				}
				LUA_TRY_CATCH
					return false;
		}

		virtual void _OnFinish()
		{
			for (int i=0; i<mWaitTimeList.size(); ++i)
			{
				if (mWaitTimeList[i])
					mWaitTimeList[i]->Finish();
			}	
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mOnFinishFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	
					(*GetScriptFactory()->mOnFinishFun)(evt);
				}
				LUA_TRY_CATCH
		}

		virtual bool OnEvent(AutoEvent &msgEvt) override
		{
			LUA_TRY_BEGIN
				if (GetScriptFactory()->mOnEventFun)
				{	
					LuaEvent *evt = GET_LUA(GetSelf());	

					LuaEvent *evt2 = GET_LUA(msgEvt);
					//evt2.mCEvent = msgEvt;

					(*GetScriptFactory()->mOnEventFun)(evt2, evt);
				}
				LUA_TRY_CATCH

					return true;
		}


		virtual void Free() override
		{
			ClearWaitEvent();
			tNode::Free();
		}

	public:
		Hand<ScriptNodeFactory> GetScriptFactory()
		{
			return GetEventFactory();
		}

	public:
		virtual void AppendWaitEvent(AutoEvent evt){ mWaitTimeList.push_back(evt); }
		void ClearWaitEvent()
		{
			for (int i=0; i<mWaitTimeList.size(); ++i)
			{
				mWaitTimeList[i]._free();
			}
			mWaitTimeList.clear();
		}

		virtual void InitData() override
		{
			tNode::InitData();
			ClearWaitEvent();
		}

	protected:
		Array<AutoEvent>	mWaitTimeList;

	};

	//-------------------------------------------------------------------------
}

#endif //_INCLUDE_SCRIPTAI_H_