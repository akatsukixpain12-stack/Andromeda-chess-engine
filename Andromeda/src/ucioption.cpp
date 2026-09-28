#include "ucioption.h"
namespace Andromeda {
Option::Option(std::string value, OnChange cb):value_(std::move(value)),callback_(std::move(cb)){}
void Option::set(const std::string& v){ value_=v; if(callback_) callback_(value_); }
Option& OptionsMap::operator[](const std::string& n){ return options_[n]; }
const Option& OptionsMap::operator[](const std::string& n) const { return options_.at(n); }
void OptionsMap::set(const std::string& n,const std::string& v){ options_[n].set(v); }
bool OptionsMap::contains(const std::string& n) const { return options_.find(n)!=options_.end(); }
}
