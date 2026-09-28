#ifndef PERFT_H_INCLUDED
#define PERFT_H_INCLUDED
#include "position.h"
#include "movegen.h"
namespace Andromeda {
inline uint64_t perft(Position& pos,int depth){
    if(depth<=0) return 1;
    Move moves[256];
    const int n=generate_legal_moves(pos,moves);
    if(depth==1) return static_cast<uint64_t>(n);
    uint64_t nodes=0;
    for(int i=0;i<n;++i){
        StateInfo si;
        if(!pos.do_move(moves[i],si)) continue;
        nodes += perft(pos,depth-1);
        pos.undo_move(moves[i]);
    }
    return nodes;
}
}
#endif
