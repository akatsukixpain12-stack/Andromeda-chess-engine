#ifndef UCIOPTION_H_INCLUDED
#define UCIOPTION_H_INCLUDED
#include <functional>
#include <map>
#include <string>
namespace Andromeda {
class Option {
public:
    using OnChange = std::function<void(const std::string&)>;
    Option() = default;
    Option(std::string value, OnChange cb = {});
    const std::string& value() const { return value_; }
    void set(const std::string& v);
private:
    std::string value_;
    OnChange callback_;
};
class OptionsMap {
public:
    Option& operator[](const std::string& name);
    const Option& operator[](const std::string& name) const;
    void set(const std::string& name,const std::string& value);
    bool contains(const std::string& name) const;
private:
    std::map<std::string,Option> options_;
};
}
#endif
