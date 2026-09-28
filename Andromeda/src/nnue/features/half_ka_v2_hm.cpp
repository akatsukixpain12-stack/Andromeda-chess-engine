#include "half_ka_v2_hm.h"
namespace Andromeda::NNUE::Features { int half_ka_index(int king,int piece,int sq){return (king&63)*512+(piece&7)*64+(sq&63);} }