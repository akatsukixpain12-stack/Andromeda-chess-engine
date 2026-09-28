#pragma once
#include "nnue_common.h"
namespace Andromeda::NNUE {
class AccumulatorStack {
public:
 AccumulatorStack(){reset();}
 void reset(){acc_.fill(0);}
 Accumulator& current(){return acc_;}
 const Accumulator& current() const{return acc_;}
 void add(int i,int v){if(i>=0&&i<HiddenSize)acc_[i]+=v;}
private: Accumulator acc_{};
};
}