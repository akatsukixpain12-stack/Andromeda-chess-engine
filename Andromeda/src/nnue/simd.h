#pragma once
#include <cstdint>
namespace Andromeda::NNUE {
inline int32_t dot_product(const int16_t* a,const int16_t* b,int n){int32_t s=0;for(int i=0;i<n;++i)s+=int32_t(a[i])*b[i];return s;}
}