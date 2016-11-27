#pragma once
#include <memory>
#include <engine/ecs/EntityManager.h>
#include <functional>
#include <engine/rendering/geometry/Mesh.h>
#include <engine/rendering/Material.h>

class Shader;

class MeshRenderer : public Component
{
    friend class MeshRenderSystem;
public:
    MeshRenderer() { }

    explicit MeshRenderer(std::shared_ptr<Mesh> mesh);

    void onShowInEditor() override;

    std::string getName() const override { return "Mesh Renderer"; };

    void render() const;
    void render(Shader* shader);

    void setMesh(std::shared_ptr<Mesh> mesh);
    void addMaterial(std::shared_ptr<Material> material);
    void setMaterial(std::shared_ptr<Material> material, uint8_t index);

    void setModelMatrix(const glm::mat4& matrix);
    void performForAllMaterials(std::function<void(Material*)> func);

    std::shared_ptr<Mesh> getMesh() const { return m_mesh; }

private:
    void bindAndRender(SubMeshIndex subMeshIdx) const;
    void ensureIntegrity() const;

private:
    std::shared_ptr<Mesh> m_mesh;
    std::vector<std::shared_ptr<Material>> m_materials;
};
