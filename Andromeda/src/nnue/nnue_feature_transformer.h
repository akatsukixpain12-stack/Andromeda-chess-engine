#pragma once
#include "nnue_common.h"
namespace Andromeda::NNUE {
class FeatureTransformer {
public:
 FeatureTransformer(){ weights_.fill(0); }
 void reset(){weights_.fill(0);}
 void add_feature(int index,int delta){if(index>=0 && index<InputFeatures) weights_[index]+=delta;}
 int32_t evaluate() const {int64_t s=0;for(auto x:weights_)s+=x;return int32_t(s);}
private:
 std::array<int32_t,InputFeatures> weights_{};
};
}