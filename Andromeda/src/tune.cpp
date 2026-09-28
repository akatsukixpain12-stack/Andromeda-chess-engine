#include "tune.h"
#include "evaluate.h"
#include <cstring>
namespace Andromeda {
void init_tuning(){ init_evaluation(); }
void set_tuning_parameter(const char* name,int value){
 if(name && std::strcmp(name,"Aggression")==0) set_aggression(value);
}
}