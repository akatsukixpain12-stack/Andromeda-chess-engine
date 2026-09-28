#pragma once
#include <cstdint>
namespace Andromeda::NNUE::Layers { inline int32_t sqr_clipped_relu(int32_t x,int32_t maxv=127){if(x<0)x=0;if(x>maxv)x=maxv;return maxv?x*x/maxv:0;} }