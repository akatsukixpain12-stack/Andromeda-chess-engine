#pragma once
#include "../type.h"
#include <cstdint>
#include <array>
namespace Andromeda::NNUE {
constexpr int InputFeatures=768;
constexpr int HiddenSize=32;
using Weight=int16_t;
using Bias=int32_t;
using Accumulator=std::array<int32_t,HiddenSize>;
inline int32_t clip_relu(int32_t x){return x<0?0:(x>127?127:x);}
}