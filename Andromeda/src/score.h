/* Andromeda - Andromeda-inspired score representation. GPLv3 applies to derived portions. */
#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED
#include <variant>
#include <utility>
#include "type.h"
namespace Andromeda {
class Position;
class Score {
public:
 struct Mate { int plies; };
 struct Tablebase { int plies; bool win; };
 struct InternalUnits { int value; };
 Score() : score(InternalUnits{0}) {}
 Score(Value v, const Position&);
 template<typename T> bool is() const { return std::holds_alternative<T>(score); }
 template<typename T> T get() const { return std::get<T>(score); }
 template<typename F> decltype(auto) visit(F&& f) const { return std::visit(std::forward<F>(f), score); }
private: std::variant<Mate,Tablebase,InternalUnits> score;
};
}
#endif
