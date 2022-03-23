#pragma once

#include "nm_math.h"

namespace nm {
    struct Mesh {
        vecXr vertices;
        vecXi faces;
        vecXr tetrahedra;
        vecXr normals;
        vecXr colors;
        vecXr wireframe_colors;
    };
}// namespace nm
