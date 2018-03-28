#ifndef _RANDTOOL_H_
#define _RANDTOOL_H_

#include "BaseCommon.h"
#include "Assertx.h"
#include "Array.h"
//-------------------------------------------------------------------------*/
// NOTE: All function include min and max value
//-------------------------------------------------------------------------
class BaseCommon_Export CRand
{
	typedef unsigned int uint32_t;

	class  SimpleRNG
	{
	private:
		uint32_t m_w;
		uint32_t m_z;
	public:
		SimpleRNG()
		{
			// These values are not magical, just the default values Marsaglia used.
			// Any pair of unsigned integers should be fine.
			m_w = 521288629;
			m_z = 362436069;
		}

		SimpleRNG(uint32_t u, uint32_t v)
		{
			SetSeed(u, v);
		}

	private:
		SimpleRNG(const SimpleRNG& porto)
		{
			SetSeed(porto.m_w, porto.m_z);
		}

		SimpleRNG& operator=(const SimpleRNG& other)
		{
			SetSeed(other.m_w, other.m_z);
			return *this;
		}

	public:
		//static SimpleRNG& Singleton()
		//{
		//	static SimpleRNG globalSingleton;
		//	return globalSingleton;
		//}

		SimpleRNG Clone() const
		{
			SimpleRNG result(*this);
			return result;
		}

	public:
		// The random generator seed can be set three ways:
		// 1) specifying two non-zero unsigned integers
		// 2) specifying one non-zero unsigned integer and taking a default value for the second
		void SetSeed(uint32_t u, uint32_t v)
		{
			if (u != 0) m_w = u; 
			if (v != 0) m_z = v;
		}

		void SetSeed(uint32_t u)
		{
			m_w = u;
		}

		void GetCurrentSeed(uint32_t& u, uint32_t& v) const
		{
			u = m_w;
			v = m_z;
		}

		// This is the heart of the generator.
		// It uses George Marsaglia's MWC algorithm to produce an unsigned integer.
		// See http://www.bobwheeler.com/statistics/Password/MarsagliaPost.txt
		uint32_t GetUint()
		{
			m_z = 36969 * (m_z & 65535) + (m_z >> 16);
			m_w = 18000 * (m_w & 65535) + (m_w >> 16);
			return (m_z << 16) + m_w;
		}

		// Produce a uniform random sample from the open interval (0, 1).
		// The method will not return either end point.
		double GetUniform()
		{
			// 0 <= u < 2^32
			uint32_t u = GetUint();
			// The magic number below is 1/(2^32 + 2).
			// The result is strictly between 0 and 1.
			return (u + 1.0) * 2.328306435454494e-10;
		}
	};

public:
    static void SetSeed(uint32_t seed);
//	static void SetSpeed(uint32_t u, uint32_t v);
//
//	static double RandZeroOne()
//	{
//		return mSimple.GetUniform();
//	}
//
//	static float Rand(float max)
//	{
//		return (float)(max * mSimple.GetUniform());
//	}
//
//	static float Rand(float min, float max)
//	{
//		return min + (float)((max-min)*mSimple.GetUniform());
//	}
//
//	static double Rand(double max)
//	{
//		return max * mSimple.GetUniform();
//	}
//
//	static double Rand(double min, double max)
//	{
//		return min + ((max-min)*mSimple.GetUniform());
//	}
//
//public:
	static int Rand(int minInt, int maxInt)
	{
		AssertEx(maxInt>=minInt, "Rand max value must more then min value");
		uint x = RandUInt((uint)(maxInt-minInt));
		return minInt + x;
	}

	// 不包含后面值
	static int RandEx(int minInt, int maxInt)
	{
		maxInt--;
		AssertEx(maxInt>=minInt, "Rand max value must more then min value");
		uint x = RandUInt((uint)(maxInt-minInt));
		return minInt + x;
	}


    static double GetUniform()
    {
        // 0 <= u < 2^16
        uint32_t u = rand();
        // The magic number below is 1/(2^16 + 1).
        // The result is strictly between 0 and 1.
        return (u + 1.0) * 1.525855623540901e-5;
    }

    static float Rand(float min, float max)
    {
        return min + (float)((max-min)*GetUniform());
    }

    static double Rand(double min, double max)
    {
        return min + ((max-min)*GetUniform());
    }

    static uint RandUInt(uint maxVal)
    {
		return Rand(maxVal);
  //      if (maxVal<=0xffff)
		//{
		//	return Rand(maxVal);
		//}

		//uint hVal =  maxVal>>16;
		//return ((Rand(hVal)<<16) + Rand((uint)0xFFFF))%(maxVal+1);
	}

	static uint Rand(uint maxInt);

	static int Rand(int maxInt)
	{
		if (maxInt<0)
			maxInt = 0;
		return Rand((uint)maxInt);
	}

//	static SimpleRNG	mSimple;
};

//-------------------------------------------------------------------------
// 随机抽取对象
//-------------------------------------------------------------------------
template<typename T>
class RandTool
{
public:
	RandTool()
		: mCurrentCount(0)
	{
	}

	void InitReset(int count)
	{
		mList.clear(false);
		mList.resize(count);
		mCurrentCount = 0;
	}

	bool empty(){ return mList.empty(); }
	int Count(){ return mCurrentCount; }

public:
	// 先压入所有随机取的对象
	void Push(const T &obj)
	{
		if (mList.size()>(size_t)mCurrentCount)
			mList[mCurrentCount] = obj;
		else
			mList.push_back(obj);
		++mCurrentCount;
	}

	bool RandPop(T &result)
	{
		if (mCurrentCount>0)
		{			
			if (mCurrentCount>1)
			{
				size_t pos = rand()%mCurrentCount;				
				result = mList[pos];
				// 把最后的一个直接移动到被取走的位置
				if (mCurrentCount-1!=pos)
					mList[pos] = mList[mCurrentCount-1];
			}
			else
				result = mList[0];

			--mCurrentCount;

			return true;
		}
		return false;
	}

protected:
	Array<T>	mList;

	int				mCurrentCount;
};


#endif //_RANDTOOL_H_