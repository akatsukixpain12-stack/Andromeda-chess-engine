#pragma once
#include <cstdint>
namespace Andromeda::NNUE {
template<class T> inline T clamp_value(T v,T lo,T hi){return v<lo?lo:(v>hi?hi:v);}
}