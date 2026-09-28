#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED
#include <cstdint>
#include <string>
#include <string_view>
namespace Andromeda {
using u64 = std::uint64_t;
using i64 = std::int64_t;
bool is_whitespace(std::string_view s);
std::string trim(std::string_view s);
std::string engine_version();
void sync_cout_start();
void sync_cout_end();
}
#endif
