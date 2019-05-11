#ifndef _int128_H
#define _int128_H

#include <assert.h>
#include <limits>
#include <stdint.h>
#include <string>
#include <vector>
#include <random>
#include <functional>
#include <algorithm>
#include <cuda_runtime.h>

#define CUDA_CALLABLE __host__ __device__

__host__ char ToLower(char c)
{
    return (c >= 'A' && c <= 'Z' ? (c - 'A') + 'a' : c);
}

inline __host__ bool IsSpace(char c) noexcept 
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

__host__ signed char HexDigit(char c)
{
    static const signed char p_util_hexdigit[256] =
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
    -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, };

    return p_util_hexdigit[(unsigned char)c];
}

template<typename T>
__host__ std::string HexStr(const T itbegin, const T itend, bool fSpaces=false)
{
    std::string rv;
    static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    rv.reserve((itend-itbegin)*3);
    for(T it = itbegin; it < itend; ++it)
    {
        unsigned char val = (unsigned char)(*it);
        if(fSpaces && (it - itbegin) % 4 == 0 && it != itbegin)
            rv.push_back(' ');
        rv.push_back(hexmap[val>>4]);
        rv.push_back(hexmap[val&15]);
    }

    return rv;
}

class int128_t
{
protected:
    uint32_t pn[4];
public:

    CUDA_CALLABLE int128_t() { }

    CUDA_CALLABLE int128_t(const int128_t& b)
    {
        memcpy(pn, b.pn, sizeof(pn));
    }

    CUDA_CALLABLE int128_t& operator=(const int128_t& b)
    {
        memcpy(pn, b.pn, sizeof(pn));
        return *this;
    }

    __host__ int128_t& operator=(const std::string &str)
    {
        SetHex(str.c_str());

        return *this;
    }

    CUDA_CALLABLE int128_t(int64_t b)
    {
        memset(pn, 0, sizeof(pn));
        
        if(b >= 0)
        {
            pn[0] = (unsigned int)b;
            pn[1] = (unsigned int)(b >> 32);
        }
        else
        {
            b = -b;
            pn[0] = (unsigned int)b;
            pn[1] = (unsigned int)(b >> 32);
            *this = -(*this);
        }
    }

    __host__ explicit int128_t(const std::string& str)
    {
        SetHex(str.c_str());
    }

    CUDA_CALLABLE inline uint32_t hash() const
    {
        uint32_t hash = 0;
        hash = (hash + (324723947 + pn[0])) ^ 93485734985;
        hash = (hash + (324723947 + pn[1])) ^ 93485734985;
        hash = (hash + (324723947 + pn[2])) ^ 93485734985;
        hash = (hash + (324723947 + pn[3])) ^ 93485734985;
        return hash;
    }

    CUDA_CALLABLE const int128_t operator~() const
    {
        int128_t ret;
        ret.pn[0] = ~pn[0];
        ret.pn[1] = ~pn[1];
        ret.pn[2] = ~pn[2];
        ret.pn[3] = ~pn[3];
        return ret;
    }

    CUDA_CALLABLE const int128_t operator-() const
    {
        int128_t ret = ~(*this);
        ++ret;
        return ret;
    }

    CUDA_CALLABLE int128_t& operator=(int64_t b)
    {
        memset(pn, 0, sizeof(pn));
        
        if(b >= 0)
        {
            pn[0] = (unsigned int)b;
            pn[1] = (unsigned int)(b >> 32);
        }
        else
        {
            b = -b;
            pn[0] = (unsigned int)b;
            pn[1] = (unsigned int)(b >> 32);
            *this = -(*this);
        }
        return *this;
    }

    CUDA_CALLABLE int128_t& operator^=(const int128_t& b)
    {
        pn[0] ^= b.pn[0];
        pn[1] ^= b.pn[1];
        pn[2] ^= b.pn[2];
        pn[3] ^= b.pn[3];
        return *this;
    }

    CUDA_CALLABLE int128_t& operator&=(const int128_t& b)
    {
        pn[0] &= b.pn[0];
        pn[1] &= b.pn[1];
        pn[2] &= b.pn[2];
        pn[3] &= b.pn[3];

        return *this;
    }

    CUDA_CALLABLE int128_t& operator|=(const int128_t& b)
    {
        pn[0] |= b.pn[0];
        pn[1] |= b.pn[1];
        pn[2] |= b.pn[2];
        pn[3] |= b.pn[3];
        return *this;
    }

    CUDA_CALLABLE int128_t& operator^=(uint64_t b)
    {
        pn[0] ^= (unsigned int)b;
        pn[1] ^= (unsigned int)(b >> 32);
        return *this;
    }

    CUDA_CALLABLE int128_t& operator|=(uint32_t b)
    {
        pn[0] |= b;
        return *this;
    }

    CUDA_CALLABLE int128_t& operator|=(uint64_t b)
    {
        pn[0] |= (unsigned int)b;
        pn[1] |= (unsigned int)(b >> 32);
        return *this;
    }

    CUDA_CALLABLE int128_t& operator<<=(unsigned int shift);
    CUDA_CALLABLE int128_t& operator>>=(unsigned int shift);

    CUDA_CALLABLE int128_t& operator%=(const int128_t& b)
    {
        *this = *this % b;
        return *this;
    }

    CUDA_CALLABLE int128_t& operator+=(const int128_t& b)
    {
        uint64_t n = 0;

        n = uint64_t(pn[0]) + b.pn[0];
        pn[0] = n;

        n = (n >> 32) + pn[1] + b.pn[1];
        pn[1] = n;

        n = (n >> 32) + pn[2] + b.pn[2];
        pn[2] = n;
        
        n = (n >> 32) + pn[3] + b.pn[3];
        pn[3] = n;
    
        return *this;
    }

    CUDA_CALLABLE int128_t& operator-=(const int128_t& b)
    {
        union {
            uint32_t lh[2];
            uint64_t n;
        };
        n = 0;

        n = (uint64_t)pn[0] - b.pn[0];
        pn[0] = lh[0];

        n = (uint64_t)pn[1] - b.pn[1] - (lh[1] != 0);
        pn[1] = lh[0];

        n = (uint64_t)pn[2] - b.pn[2] - (lh[1] != 0);
        pn[2] = lh[0];
        
        n = (uint64_t)pn[3] - b.pn[3] - (lh[1] != 0);
        pn[3] = lh[0];
    
        return *this;
    }

    CUDA_CALLABLE int128_t& operator*=(const int128_t& b);
    CUDA_CALLABLE int128_t& operator/=(const int128_t& b);

    // prefix operator
    CUDA_CALLABLE int128_t& operator++()
    {
        int i = 0;
        while (i < 4 && ++pn[i] == 0)
            ++i;
        return *this;
    }

    // postfix operator
    CUDA_CALLABLE const int128_t operator++(int)
    {
        const int128_t ret = *this;
        ++(*this);
        return ret;
    }

    // prefix operator
    CUDA_CALLABLE int128_t& operator--()
    {
        int i = 0;
        while (i < 4 && --pn[i] == 0xffffffff)
            ++i;
        return *this;
    }

    // postfix operator
    CUDA_CALLABLE const int128_t operator--(int)
    {
        const int128_t ret = *this;
        --(*this);
        return ret;
    }

    CUDA_CALLABLE bool isLessZero() const
    {
        return pn[3] & 0x80000000;
    }

    CUDA_CALLABLE friend inline const int128_t operator%(const int128_t& a, const int128_t& b)
    {
        return a - b * ( a / b );
    }

    CUDA_CALLABLE friend inline const int128_t operator+(const int128_t& a, const int128_t& b) { return int128_t(a) += b; }
    CUDA_CALLABLE friend inline const int128_t operator-(const int128_t& a, const int128_t& b) { return int128_t(a) -= b; }
    CUDA_CALLABLE friend inline const int128_t operator*(const int128_t& a, const int128_t& b) { return int128_t(a) *= b; }
    CUDA_CALLABLE friend inline const int128_t operator/(const int128_t& a, const int128_t& b) { return int128_t(a) /= b; }
    CUDA_CALLABLE friend inline const int128_t operator|(const int128_t& a, const int128_t& b) { return int128_t(a) |= b; }
    CUDA_CALLABLE friend inline const int128_t operator&(const int128_t& a, const int128_t& b) { return int128_t(a) &= b; }
    CUDA_CALLABLE friend inline uint32_t operator&(const int128_t& a, const uint32_t b) { return a.pn[0] & b; }
    CUDA_CALLABLE friend inline const int128_t operator^(const int128_t& a, const int128_t& b) { return int128_t(a) ^= b; }
    CUDA_CALLABLE friend inline const int128_t operator>>(const int128_t& a, unsigned shift) { return int128_t(a) >>= shift; }
    CUDA_CALLABLE friend inline const int128_t operator<<(const int128_t& a, unsigned shift) { return int128_t(a) <<= shift; }

    CUDA_CALLABLE friend inline bool operator==(const int128_t& a, const int128_t& b)
    {
        return (a.pn[0] == b.pn[0]) && (a.pn[1] == b.pn[1]) && (a.pn[2] == b.pn[2]) && (a.pn[3] == b.pn[3]);
    }

    CUDA_CALLABLE friend inline bool operator!=(const int128_t& a, const int128_t& b) { return !(a == b); }

    CUDA_CALLABLE friend inline bool operator<(const int128_t& a, const int128_t& b)
    {
        const bool lhsSign = a.isLessZero();
        const bool rhsSign = b.isLessZero();

        if( lhsSign != rhsSign )
            return lhsSign > rhsSign;
    
        int i = 3;
        for(; a.pn[i] == b.pn[i] && i > 0; --i)
        {
            // do nothing;
        }

        if(a.pn[i] < b.pn[i])
            return !lhsSign;
        else
            return lhsSign;
    }

    CUDA_CALLABLE friend inline bool operator>(const int128_t& a, const int128_t& b)
    {
        const bool lhsSign = a.isLessZero();
        const bool rhsSign = b.isLessZero();

        if( lhsSign != rhsSign )
            return lhsSign < rhsSign;

        int i = 3;
        for(; a.pn[i] == b.pn[i] && i > 0; --i)
        {
            // do nothing;
        }

        if(a.pn[i] > b.pn[i])
            return !lhsSign;
        else
            return lhsSign;
    }

    CUDA_CALLABLE friend inline bool operator>=(const int128_t& a, const int128_t& b) { return !(a < b); }
    CUDA_CALLABLE friend inline bool operator<=(const int128_t& a, const int128_t& b) { return !(a > b); }
    __host__ friend inline std::ostream& operator<<(std::ostream& out, const int128_t &a) { return out << a.GetHex(); }

    __host__ std::string GetHex() const;
    void SetHex(const char* psz);

    CUDA_CALLABLE inline unsigned bitlength() const
    {
        uint64_t *tmp = (uint64_t *)pn;
#ifdef __CUDA_ARCH__
        if(tmp[1])
            return 129 - __clzll(tmp[1]);
        else if(tmp[0])
            return 65 - __clzll(tmp[0]);
        else 
            return 0;
#else
        if(tmp[1])
            return 129 - __builtin_clzll(tmp[1]);
        else if(tmp[0])
            return 65 - __builtin_clzll(tmp[0]);
        else return 0;
#endif
    }

    __host__ const int128_t& random( const int128_t &mod );

    // Works for values greater then 0
    CUDA_CALLABLE void div(const int128_t &a, const int128_t &b)
    {
        memset(pn, 0, sizeof(pn));

        int128_t num = a;
        int128_t div = b;
        
        int shift = num.bitlength() - div.bitlength();
        if(shift < 0)
            return;

        div <<= shift; // shift so that div and num align
        
        for(; shift >= 0; --shift)
        {
            if(num >= div)
            {
                num -= div;
                pn[shift >> 5] |= (1 << (shift & 31)); // set a bit of the result
            }
            div >>= 1; // shift back
        }
    }
};

CUDA_CALLABLE int128_t& int128_t::operator<<=(unsigned int shift)
{
    if(shift == 0)
        return *this;

    const int k = shift >> 5;

    if(k != 0)
    {
        for(int i = 3; i >= k; --i)
        {
            pn[i] = pn[i - k];
        }
        memset(pn, 0, k);
    }

    shift %= 32;
    if(shift == 0)
        return *this;

    unsigned begin = 0, end = 0;
    for(int i = k; i < 4; ++i)
    {
        begin = pn[i];
        pn[i] = (begin << shift) | (end >> (32 - shift));
        end = begin;
    }

    return *this;
}

CUDA_CALLABLE int128_t& int128_t::operator>>=(unsigned int shift)
{
    if(shift == 0)
        return *this;

    const int k = shift >> 5;
    if(k != 0)
    {
        for(int i = 0; i < 4 - k; ++i)
        {
            pn[i] = pn[i + k];
        }
        memset(pn + k, 0, 4 - k);
    }

    shift %= 32;
    if(shift == 0)
        return *this;

    unsigned begin = 0, end = 0;
    for(int i = 3 - k; i >= 0; --i)
    {
        begin = pn[i];
        pn[i] = (begin >> shift) | (end << (32 - shift));
        end = begin;
    }

    return *this;
}

int128_t& int128_t::operator*=(const int128_t& b)
{
    int128_t a; // need to make "in place"

    uint64_t n = 0, tmp = (uint64_t)pn[0];
    n = tmp * b.pn[0];
    a.pn[0] = n;

    n = (n >> 32) + tmp * b.pn[1];
    a.pn[1] = n;

    n = (n >> 32) + tmp * b.pn[2];
    a.pn[2] = n;

    n = (n >> 32) + tmp * b.pn[3];
    a.pn[3] = n;

    tmp = (uint64_t)pn[1];
    n = a.pn[1] + tmp * b.pn[0];
    a.pn[1] = n;

    n = (n >> 32) + a.pn[2] + tmp * b.pn[1];
    a.pn[2] = n;

    n = (n >> 32) + a.pn[3] + tmp * b.pn[2];
    a.pn[3] = n;

    tmp = (uint64_t)pn[2];
    n = a.pn[2] + tmp * b.pn[0];
    a.pn[2] = n;

    n = (n >> 32) + a.pn[3] + tmp * b.pn[1];
    a.pn[3] = n;


    n = a.pn[3] + (uint64_t)pn[3] * b.pn[0];
    a.pn[3] = n;

    *this = a;
    return *this;
}

// Works for values greater then 0
CUDA_CALLABLE int128_t& int128_t::operator/=(const int128_t& b)
{
    int128_t div = b;
    int128_t num = (*this);

    memset(pn, 0, sizeof(pn));

    int num_bits = num.bitlength();
    int div_bits = div.bitlength();
    
    if (div_bits > num_bits) // the result is certainly 0.
        return *this;
    
    int shift = num_bits - div_bits;
    div <<= shift; // shift so that div and num align.
    
    while(shift >= 0)
    {
        if( num >= div )
        {
            num -= div;
            pn[shift >> 5] |= (1 << (shift & 31)); // set a bit of the result.
        }
        div >>= 1; // shift back.
        --shift;
    }

    return (*this);
}

__host__ std::string int128_t::GetHex() const
{
    return HexStr(std::reverse_iterator<const uint8_t*>((const uint8_t*)pn + sizeof(pn)), std::reverse_iterator<const uint8_t*>((const uint8_t*)pn), true);
}

void int128_t::SetHex(const char* psz)
{
    memset(pn, 0, sizeof(pn));

    // skip leading spaces
    while (IsSpace(*psz))
        psz++;

    // skip 0x
    if (psz[0] == '0' && ToLower(psz[1]) == 'x')
        psz += 2;

    // hex string to uint
    const char* pbegin = psz;
    while (::HexDigit(*psz) != -1)
        psz++;
    psz--;
    unsigned char* p1 = (unsigned char*)pn;
    unsigned char* pend = p1 + 16;
    while (psz >= pbegin && p1 < pend)
    {
        *p1 = ::HexDigit(*psz--);
        if (psz >= pbegin)
        {
            *p1 |= ((unsigned char)::HexDigit(*psz--) << 4);
            p1++;
        }
    }
}

const int128_t& int128_t::random( const int128_t &mod )
{
    static std::mt19937 gen( time(NULL) );
    std::generate(pn, pn + 4, std::ref(gen));
    pn[3] &= ~(1U << 31);
    if( *this >= mod )
        *this %= mod;
    return *this;
}

#endif // _int128_H
