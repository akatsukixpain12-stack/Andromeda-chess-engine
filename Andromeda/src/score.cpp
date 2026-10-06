/* Andromeda - Andromeda-inspired score conversion. */
#include "score.h"
#include <cassert>
#include <cmath>
#include "position.h"
namespace Andromeda {
Score::Score(Value v,const Position&) {
 assert(-VALUE_INFINITE < v && v < VALUE_INFINITE);
 if (std::abs(v) < VALUE_MATE_IN_MAX_PLY)
   score = InternalUnits{v};
 else {
   int d = VALUE_MATE - std::abs(v);
   score = (v > 0) ? Mate{d} : Mate{-d};
 }
}
}
