#pragma once
#include "position.h"

namespace Andromeda {

Value evaluate(const Position& pos);

void init_evaluation();
void set_aggression(int value);
int aggression();

} // namespace Andromeda
