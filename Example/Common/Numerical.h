#ifndef _COMMON_NUMERICAL_H_
#define _COMMON_NUMERICAL_H_
#include <limits>
#include <cstdint>

inline uint32_t CastToUInt32(double n)
{
    if (n < 0.0)
        return 0;

    if (n > (double)numeric_limits<uint32_t>().max())
        return numeric_limits<uint32_t>().max();

    return static_cast<uint32_t>(n);
}

inline int32_t CastToInt32(double n)
{
    if (n < (double)numeric_limits<int32_t>().min())
        return numeric_limits<int32_t>().min();

    if (n > (double)numeric_limits<int32_t>().max())
        return numeric_limits<int32_t>().max();

    return static_cast<int32_t>(n);
}

inline int64_t CastToInt64(double n)
{
    if (n < (double)numeric_limits<int64_t>().min())
        return numeric_limits<int64_t>().min();

    if (n > (double)numeric_limits<int64_t>().max())
        return numeric_limits<int64_t>().max();

    return static_cast<int64_t>(n);
}

inline int32_t BoundedAddInt32(int32_t n, int32_t m)
{
    if (m > 0 && n + m < n)
        return numeric_limits<int32_t>().max();
    else if (m < 0 && n + m > n)
        return numeric_limits<int32_t>().min();
    else
        return n + m;
}

inline uint32_t BoundedAddUint32(uint32_t n, uint32_t m)
{
    if (n >= numeric_limits<uint32_t>().max() || m >= numeric_limits<uint32_t>().max())
        return numeric_limits<uint32_t>().max();

    if (n > numeric_limits<uint32_t>().max() - m)
        return numeric_limits<uint32_t>().max();

    return n + m;
}

#endif // !_COMMON_NUMERICAL_H_
