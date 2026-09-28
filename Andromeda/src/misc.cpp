#include "misc.h"
#include <algorithm>
#include <cctype>
#include <iostream>
namespace Andromeda {
bool is_whitespace(std::string_view s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c){ return std::isspace(c) != 0; });
}
std::string trim(std::string_view s) {
    std::size_t a=0,b=s.size();
    while(a<b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while(b>a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return std::string(s.substr(a,b-a));
}
std::string engine_version() { return "Andromeda 4.0"; }
void sync_cout_start() { std::cout.flush(); }
void sync_cout_end() { std::cout.flush(); }
}
