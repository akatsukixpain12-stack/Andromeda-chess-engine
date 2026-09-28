#pragma once
#include <array>
#include <cstdint>
namespace Andromeda::NNUE::Layers {
template<int In,int Out> struct AffineTransform {
 std::array<int16_t,In*Out> weights{}; std::array<int32_t,Out> bias{};
 template<class Input> std::array<int32_t,Out> propagate(const Input& x) const {
  std::array<int32_t,Out> y=bias; for(int o=0;o<Out;++o)for(int i=0;i<In;++i)y[o]+=int32_t(weights[o*In+i])*x[i]; return y;
 }
};
}