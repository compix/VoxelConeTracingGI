#pragma once
#include "SimpleMeshRenderer.h"
#include <memory>
#include "MeshRenderer.h"

class MeshRenderers
{
public:
    static void init();

    static std::shared_ptr<SimpleMeshRenderer> fullscreenQuad() { return m_fullscreenQuad; }

    static std::shared_ptr<SimpleMeshRenderer> quad() { return m_quad; }

    static std::shared_ptr<SimpleMeshRenderer> line() { return m_line; }

    static std::shared_ptr<SimpleMeshRenderer> nonFilledBox() { return m_nonFilledBox; }

    static std::shared_ptr<SimpleMeshRenderer> triangle() { return m_triangle; }

    static std::shared_ptr<MeshRenderer> box() { return m_box; }

    static std::shared_ptr<MeshRenderer> arrowHead() { return m_arrowHead; }

    static std::shared_ptr<MeshRenderer> cylinder() { return m_cylinder; }

    static std::shared_ptr<MeshRenderer> sphere() { return m_sphere; }
private:
    static void loadQuad();
    static void loadFullscreenQuad();
    static void loadLine();
    static void loadNonFilledCube();
    static void loadTriangle();

    static std::shared_ptr<SimpleMeshRenderer> m_quad;
    static std::shared_ptr<SimpleMeshRenderer> m_fullscreenQuad;
    static std::shared_ptr<SimpleMeshRenderer> m_line;
    static std::shared_ptr<SimpleMeshRenderer> m_nonFilledBox;
    static std::shared_ptr<SimpleMeshRenderer> m_triangle;

    static std::shared_ptr<MeshRenderer> m_box;
    static std::shared_ptr<MeshRenderer> m_arrowHead;
    static std::shared_ptr<MeshRenderer> m_cylinder;
    static std::shared_ptr<MeshRenderer> m_sphere;
};
