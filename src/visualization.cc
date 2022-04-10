#include "visualization.h"
#include "mouse_control.h"
#include "renderer_utils.h"
#include <spdlog/spdlog.h>

namespace nm {
    constexpr real kPickingTolerance = 0.1;

    igl::opengl::glfw::Viewer viewer;
    igl::opengl::glfw::imgui::ImGuiPlugin plugin;
    igl::opengl::glfw::imgui::ImGuiMenu menu;

    int pickedVertex = -1;

    bool isMouseDragging;

    vec3r mouseWindow;
    vec3r mouseDrag;
    vec3r mouseWorld;
    vec3r mouseDragWorld;

    Mesh mesh;

    auto getMeshInstance() -> Mesh & { return mesh; }
    auto getViewerInstance() -> igl::opengl::glfw::Viewer & { return viewer; }

    void drawGrid() {
        spdlog::debug("Drawing grid");
        matXr points;
        matXi edges;
        nm::makeRenderableGrid(10.0, 100, points, edges, 0.0);
        viewer.data().set_edges(points, edges, Rowvec3r(1.0, 1.0, 1.0));
        viewer.core().background_color = vec4<float>(0.15, 0.15, 0.15, 1.0);
    }

    void updateVertexPositions(const vecXr &pos) {
        // Update vertex positions
        for (unsigned int ii = 0; ii < getVertices(viewer).rows(); ++ii) {
            mesh.vertices.row(ii) = pos.segment<3>(3 * ii).transpose();
        }

        viewer.data_list[0].V = mesh.vertices;

        // Tell viewer to update
        viewer.data_list[0].dirty |= igl::opengl::MeshGL::DIRTY_POSITION;
    }

    void setMesh(const std::string &filename, bool tetrahedralize) {
        auto &[_id, V, F, _T] = getMeshInstance();
        igl::read_triangle_mesh(filename, V, F);
        if (tetrahedralize) { tetrahedralizeMesh(getMeshInstance()); }
        viewer.data().set_mesh(V, F);
    }

    auto drawCallback(igl::opengl::glfw::Viewer &viewerInstance) -> bool { return false; }

    auto mouseDown(igl::opengl::glfw::Viewer &viewerInstance, int x, int y) -> bool {
        mouseWindow = vec3r(viewerInstance.current_mouse_x,
                            viewerInstance.core().viewport(3) - viewerInstance.current_mouse_y, 0.);

        // Project the ray from the mouse and get the mouse pos in world space.
        igl::unproject(mouseWindow, viewerInstance.core().view, viewerInstance.core().proj,
                       viewerInstance.core().viewport, mouseWorld);

        // Compute the vertices in world space nearest to the mouse cursor position.
        pickedVertex = pickNearestVertex(
                mouseWindow, viewerInstance.core().view.cast<real>(), viewerInstance.core().proj.cast<real>(),
                viewerInstance.core().viewport.cast<real>(), mesh.vertices, mesh.faces, kPickingTolerance);

        isMouseDragging = true;

        return false;
    }

    auto mouseUp(igl::opengl::glfw::Viewer &viewerInstance, int x, int y) -> bool {
        isMouseDragging = false;
        pickedVertex = -1;
        mouseDragWorld.setZero();
        return false;
    }

    auto mouseMove(igl::opengl::glfw::Viewer &viewerInstance, int x, int y) -> bool {
        mouseDrag = vec3r(viewerInstance.current_mouse_x,
                          viewerInstance.core().viewport(3) - viewerInstance.current_mouse_y, 0.0) -
                    mouseWindow;
        mouseWindow = vec3r(viewerInstance.current_mouse_x,
                            viewerInstance.core().viewport(3) - viewerInstance.current_mouse_y, 0.0);
        mouseDragWorld -= mouseWorld;
        igl::unproject(mouseWindow, viewerInstance.core().view, viewerInstance.core().proj,
                       viewerInstance.core().viewport, mouseWorld);

        if (isMouseDragging && pickedVertex >= 0) { return true; }

        return false;
    }

    void callbackDrawViewerMenu() { menu.draw_viewer_menu(); }

    auto getFaces(const igl::opengl::glfw::Viewer &viewerInstance) -> matXi { return viewerInstance.data_list.at(0).F; }

    auto getVertices(const igl::opengl::glfw::Viewer &viewerInstance) -> matXr {
        return viewerInstance.data_list.at(0).V;
    }

    auto initialize() -> bool {
        spdlog::info("Initializing");
        drawGrid();


        //        spdlog::info("Loading mesh");
        //        mesh = std::make_unique<Mesh>("assets/bunny.obj");
        //        if (!tetrahedralizeMesh(mesh.get())) {
        //            spdlog::error("Tetrahedralization failed. Exiting");
        //            return false;
        //        }
        //
        //        simulationState =
        //                simulationStateFactory(mesh.vertices, mesh.tetrahedra, youngsModulus, poissonsRatio, dt, density);
        //
        //        simulationState.setSimulationConstraint(simulationConstraintFactory(mesh.vertices, 0.1));
        //
        //        auto simThread = std::thread(simulationCallback);
        //        simThread.detach();

        plugin.widgets.push_back(&menu);
        viewer.plugins.push_back(&plugin);
        viewer.callback_post_draw = &drawCallback;
        viewer.callback_mouse_down = &mouseDown;
        viewer.callback_mouse_move = &mouseMove;
        viewer.callback_mouse_up = &mouseUp;
        menu.callback_draw_viewer_menu = &callbackDrawViewerMenu;
        setMesh("assets/bunny.obj");
        return true;
    }

    auto launch() -> int { return viewer.launch(); }
}// namespace nm