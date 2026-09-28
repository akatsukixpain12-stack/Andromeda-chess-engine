#include "nnue_common.h"
namespace Andromeda::NNUE {
static bool initialized=false;
void init_network(){initialized=true;}
bool network_initialized(){return initialized;}
}