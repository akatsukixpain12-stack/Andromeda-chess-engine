#pragma once
#include <cstdint>
namespace Andromeda::NNUE::Layers { inline int32_t clipped_relu(int32_t x,int32_t maxv=127){return x<0?0:(x>maxv?maxv:x);} }