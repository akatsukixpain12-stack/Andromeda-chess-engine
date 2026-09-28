#pragma once
#include "affine_transform.h"
namespace Andromeda::NNUE::Layers { template<int In,int Out> using SparseAffineTransform=AffineTransform<In,Out>; }