
#ifndef _INCLUDE_OBJECTID_H_
#define _INCLUDE_OBJECTID_H_

typedef int	ID;
typedef	unsigned char	TD;

//union Code
//{
//public:
//	struct
//	{
//		ID mIndex       : 24;	  // 低24位保存索引编号   最大 1677 7215
//		ID mThread      : 8;     // 高8位表示线程索引    最大 255
//	};
//	int			mCode;
//
//public:
//	Code()
//		: mCode(0)
//	{
//
//	}
//	Code(int nCode)
//		: mCode(nCode)
//	{
//	}
//
//	Code(int nIndex, int nThread)
//		: mIndex(nIndex)
//		, mThread(nThread)
//	{
//	}
//
//	Code(const Code& other)
//		: mCode(other.mCode)
//	{
//	}
//
//public:
//	operator ID () const { return mIndex; }		// 转型为索引编号, 如果变量类型为无符号数,则直接可以取到索引编号
//	operator TD () const { return mThread; }		// 自动返回线程
//	operator int () const { return mCode; }		// 转型为整数, 如果变量类型为整数, 则直接可以转递所有值
//	Code& operator = (int nCode){ mCode = nCode; return *this; }			//全值接收
//	Code& operator = (ID uIndex){ mIndex = uIndex; return *this; }			//直接接收索引
//	Code& operator = (TD uThread){ mThread = uThread; return *this; }		//直接接收线程索引
//};

//typedef Code		CODE;

typedef int		CODE;



//union KEY
//{
//public:
//	struct
//	{		
//		int mRight;       		  
//		int mLeft;    
//	};
//	unsigned __int64	mValue;
//
//public:
//	KEY()
//		: mValue(0)
//	{
//
//	}
//	KEY(unsigned __int64 nV)
//		: mValue(nV)
//	{
//	}
//
//	KEY(int nK1, int nK2)
//		: mRight(nK2)
//		, mLeft(nK1)
//	{
//	}
//
//	KEY(const KEY& other)
//		: mValue(other.mValue)
//	{
//	}
//
//public:
//	int DBID(){ return mLeft; }
//	int DB(){ return mRight; }
//
//	operator unsigned __int64 () { return mValue; }
//};



#endif //_INCLUDE_OBJECTID_H_