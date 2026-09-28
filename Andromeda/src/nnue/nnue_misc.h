#pragma once
#include <cstdint>
namespace Andromeda::NNUE {
bool load_network(const char* path);
std::uint64_t network_checksum();
}