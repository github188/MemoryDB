#ifndef _INCLUDE_AI_H_
#define _INCLUDE_AI_H_

#include "Hand.h"
#include "AnyData.h"
#include "CEvent.h"
#include "EventCenter.h"
/*/-------------------------------------------------------------------------/
AI原理： 心智图原理
1 每个节点都具有执行，且只执行一件事件
2 节点具有激活，退出，分析功能
3 节点激活前，必须由当前活动的节点按顺序递归退回移动到需要激活的节点
4 激活后，即执行
5 执行后，进入分析
6 对象保存的即是当前活动的节点
7 每个节点内都存在共享的全局的数据体, 且具有自身私有的数据
//-------------------------------------------------------------------------*/

class tObject;

namespace AI
{

	//-------------------------------------------------------------------------*/
	// AI数据内保存主体对象指针，节点内保存数据体，所以当主体丢弃节点时，必须先对主体数据中的指针设置为空
	class tNode;
	class tAIData : public NiceData
	{
		friend class tNode;
		friend class tAI;
	public:
		tObject* GetOwner() { return mpOwner; }

	public:
		tAIData()
			: mpOwner(NULL)
		{
			 
		}

	protected:
		tObject	*mpOwner;
	};
	typedef Auto<tAIData> AIData;

	//-------------------------------------------------------------------------*/
	class tNode : public Logic::CEvent
	{
		friend class tAI;
		typedef Hand<tNode> ANode;
	public:
		virtual void start() = 0;
		virtual bool needStart() = 0;
		virtual bool OnTrigger(int eventType, NiceData *pData) { return false; }

		virtual void OnDead() {}
		virtual void OnHit(int attacker) {}
		virtual void OnHpChanged(int oldHp) {}

		virtual void think()
		{
			ANode needStartAI = thinkStart();
			if (needStartAI)
				needStartAI->DoEvent(false);
			else
			{
				needStartAI = thinkUpStart();
				if (needStartAI)
					needStartAI->DoEvent(false);
			}
		}

		virtual void Trigger(int eventType, NiceData *pData)
		{
			if (!_Trigger(eventType, pData, true))
				_Trigger(eventType, pData, false);			
		}

		virtual bool _Trigger(int eventType, NiceData *pData, bool up)
		{
			if (OnTrigger(eventType, pData))
				return true;

			if (up)
			{
				if (mParent)
				{				
					return mParent->_Trigger(eventType, pData, up);
				}
			}
			else
			{
				for (auto it=mChildrenList.begin(); it; ++it)
				{
					ANode n = *it;
					if (n->_Trigger(eventType, pData, up))
						return true;
				}					
			}
			return false;
		}

		virtual bool enable(){ return mAIData->GetOwner()!=NULL; }

		virtual bool _DoEvent() override
		{
			if (GetOwner()==NULL)
			{
				Free();
				return false;
			}
			start();
			return true;
		}

		virtual void _OnFinish() override
		{
			think();
		}

	public:
		// 开启准备好的节点, 必须在当前能够递归上去，或子节点中的
		ANode StartUpNode(const char *szNodeName)
		{
			if (mParent)
			{
				if (strcmp(mParent->GetEventName(), szNodeName)==0)
				{
					mParent->DoEvent(false);
					return mParent;
				}
				return mParent->StartUpNode(szNodeName);
			}

			return ANode();
		}

		ANode StartNode(const char *szNodeName)
		{
			for (auto it=mChildrenList.begin(); it; ++it)
			{
				ANode n = *it;
				if (strcmp(n->GetEventName(), szNodeName)==0)
				{
					n->DoEvent(false);
					return n;
				}
				ANode re = n->StartNode(szNodeName);
				if (re)
					return re;
			}					
			return ANode();
		}

		// 分析向下需要执行的AI
		ANode thinkStart()
		{
			for (auto it=mChildrenList.begin(); it; ++it)
			{
				ANode n = *it;
				if (n->needStart())
				{					
					return n;
				}
				ANode re = n->thinkStart();
				if (re)
					return re;
			}					
			return ANode();
		}

		// 分析向上需要执行的AI
		ANode thinkUpStart()
		{
			if (mParent)
			{
				if (mParent->needStart())
				{ 
					return mParent;
				}
				return mParent->thinkUpStart();
			}

			return ANode();
		}

		tObject* GetOwner() { return mAIData->GetOwner(); }

		// 对象中的当前节点在对象释放时必须执行此功能, 保证主体对象指针正确
		void Destory()
		{
			mAIData->mpOwner = NULL;
		}

	public:
		void Init(AIData data)
		{
			mAIData = data;
		}
		bool AppendChildren(const char *szNodeName, bool bRelpace)
		{
			if (!bRelpace && mChildrenList.exist(szNodeName))
				return false;
			Hand<tNode> node = StartEvent(szNodeName);
			if (node)
			{
				AppendChildren(node);
				return true;
			}
			return false;
		}

		void AppendChildren(Hand<tNode> children)
		{
			children->mAIData = mAIData;
			mChildrenList.insert(children->GetEventName(), children);
		}

	protected:
		Hand<tNode>	mParent;

		EasyHash<AString, Hand<tNode>>	mChildrenList;

		AIData		mAIData;
	};

	typedef Hand<tNode> ANode;
	//-------------------------------------------------------------------------*/
	// AI 应用
	//-------------------------------------------------------------------------*/
	class tAI : public MemBase
	{
	public:
		void Start(const char *szAIName, Logic::tEventCenter *pCenter)
		{
			Stop();
			mAINode = pCenter->StartEvent(szAIName);
			if (mAINode)
			{
				mAINode->mAIData = mAIData;
				mAINode->DoEvent(false);
			}
		}

		void Stop()
		{
			if (mAINode)
			{
				mAINode->Finish();
				mAINode->Free();
				mAINode.setNull();
			}
		}

	public:
		tAI(tObject *pOwner)
		{
			mAIData = MEM_NEW tAIData();
			mAIData->mpOwner = pOwner;
		}

		tAI(AIData aiData)
		{
			mAIData = aiData;
		}

		~tAI()
		{
			Stop();
		}

	public:
		AIData			mAIData;
		ANode			mAINode;
	};
	//-------------------------------------------------------------------------*/

}


#endif //_INCLUDE_AI_H_