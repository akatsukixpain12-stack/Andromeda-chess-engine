#pragma once
namespace Andromeda::NNUE {
void init_network();
bool network_initialized();
int architecture_input_size();
int architecture_hidden_size();
}