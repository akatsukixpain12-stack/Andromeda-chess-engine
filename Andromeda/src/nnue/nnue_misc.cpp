#include "nnue_misc.h"
#include <fstream>
namespace Andromeda::NNUE {
bool load_network(const char* path){std::ifstream f(path,std::ios::binary);return f.good();}
std::uint64_t network_checksum(){return 0x414E44524F4D4544ULL;}
}