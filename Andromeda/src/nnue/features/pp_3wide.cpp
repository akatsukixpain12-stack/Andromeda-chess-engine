#include "pp_3wide.h"
namespace Andromeda::NNUE::Features { int pp3_index(int piece,int sq){return (piece&7)*64+(sq&63);} }