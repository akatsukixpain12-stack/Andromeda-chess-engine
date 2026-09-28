#pragma once
#include "../type.h"
namespace Andromeda::Syzygy {
struct ProbeResult { bool available=false; Value score=VALUE_DRAW; int dtz=0; };
bool init(const char* path);
ProbeResult probe_wdl(const class Position& pos);
ProbeResult probe_dtz(const class Position& pos);
}