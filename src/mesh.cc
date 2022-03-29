#include "mesh.h"
#include <igl/read_triangle_mesh.h>
#include <igl/unique_simplices.h>
#include <igl/copyleft/tetgen/tetrahedralize.h>

#include <utility>

namespace nm {
    Mesh::Mesh(const std::string &mesh_file) {
        igl::read_triangle_mesh(mesh_file, vertices, faces);
    }

    void tetrahedralizeMesh(Mesh *mesh, const char* flags) {
        matXr TV;
        matXi F, TF, TT;
        igl::unique_simplices(mesh->faces, F);

        const TetgenResult res = static_cast<const TetgenResult>(
                igl::copyleft::tetgen::tetrahedralize(mesh->vertices, F, flags, TV, TT, TF));

        if (res != 0) {
            switch (res) {
                case kTetgenFailedToConvert:
                    std::cerr << "Internal igl error occurred" << std::endl;
                case kTetgenCrashed:
                    std::cerr << "Tetgen crashed" << std::endl;
                case kTetgenFailedToCreateTets:
                    std::cerr << "Tets could not be created for some reason" << std::endl;
                case kTetgenFailedToConvertToMatrix:
                    std::cerr << "Matrix conversion failed for some reason" << std::endl;
                default:
                    break;
            }
        }

        mesh->vertices = TV;
        mesh->faces = TF;
        mesh->tetrahedra = TT;
    }
}// namespace nm