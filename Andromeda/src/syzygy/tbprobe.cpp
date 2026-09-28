#include "tbprobe.h"
namespace Andromeda::Syzygy {
static bool ready=false;
bool init(const char*){ready=false;return ready;}
ProbeResult probe_wdl(const Position&){return {};}
ProbeResult probe_dtz(const Position&){return {};}
}