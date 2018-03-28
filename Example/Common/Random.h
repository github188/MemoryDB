#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <stdint.h>

/// <summary>
/// SimpleRNG is a simple random number generator based on 
/// George Marsaglia's MWC (multiply with carry) generator.
/// Although it is very simple, it passes Marsaglia's DIEHARD
/// series of random number generator tests.
/// 
/// Written by John D. Cook 
/// http://www.johndcook.com
/// </summary>
namespace SimpleRandom
{
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
        //    static SimpleRNG globalSingleton;
        //    return globalSingleton;
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

        int EvenlyDistribute(int exclusiveMax, bool useHighOrderBit =  false)
        {
            uint32_t n = 0xFFFFFFFF - (0xFFFFFFFF % (uint32_t)exclusiveMax);
            uint32_t u = 0;
            do 
            {
                u = GetUint();
            } while (u > n);

            if (useHighOrderBit)
                return (int)((double)u * ((double)exclusiveMax / (double)n));
            else
                return (int)(u % (uint32_t)exclusiveMax);
        }
    };

    //inline
    //void SetSeed(uint32_t u, uint32_t v)
    //{
    //    SimpleRNG::Singleton().SetSeed(u, v);
    //}

    //inline
    //void SetSeed(uint32_t u)
    //{
    //    SimpleRNG::Singleton().SetSeed(u);
    //}

    //inline
    //void GetCurrentSeed(uint32_t& u, uint32_t& v)
    //{
    //    SimpleRNG::Singleton().GetCurrentSeed(u, v);
    //}

    //inline
    //uint32_t GetUint()
    //{
    //    return SimpleRNG::Singleton().GetUint();
    //}

    //inline
    //double GetUniform()
    //{
    //    return SimpleRNG::Singleton().GetUniform();
    //}
}

#endif //_RANDOM_H_