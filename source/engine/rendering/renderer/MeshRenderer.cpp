#include "MeshRenderer.h"
#include <imgui/imgui.h>

MeshRenderer::MeshRenderer(std::shared_ptr<Mesh> mesh)
    : m_mesh(mesh) {}

void MeshRenderer::onShowInEditor()
{
    for (size_t i = 0; i < m_materials.size(); ++i)
    {
        std::string materialName = "Material" + std::to_string(i);
        auto& material = m_materials[i];

        if (ImGui::TreeNode(materialName.c_str()))
        {
            for (auto& pair : material->m_floatMap)
            {
                auto desc = EditableMaterialProperties::getDesc(pair.first);
                if (desc)
                    ImGui::SliderFloat(pair.first.c_str(), &pair.second, desc->min, desc->max);
            }

            for (auto& pair : material->m_vec2Map)
            {
                auto desc = EditableMaterialProperties::getDesc(pair.first);
                if (desc)
                    ImGui::SliderFloat2(pair.first.c_str(), &pair.second[0], desc->min, desc->max);
            }

            for (auto& pair : material->m_vec3Map)
            {
                auto desc = EditableMaterialProperties::getDesc(pair.first);
                if (desc)
                {
                    if (desc->isColor)
                        ImGui::ColorEdit3(pair.first.c_str(), &pair.second[0]);
                    else
                        ImGui::SliderFloat3(pair.first.c_str(), &pair.second[0], desc->min, desc->max);
                }
            }

            for (auto& pair : material->m_vec4Map)
            {
                auto desc = EditableMaterialProperties::getDesc(pair.first);
                if (desc)
                {
                    if (desc->isColor)
                        ImGui::ColorEdit4(pair.first.c_str(), &pair.second[0]);
                    else
                        ImGui::SliderFloat4(pair.first.c_str(), &pair.second[0], desc->min, desc->max);
                }
            }

            ImGui::TreePop();
        }
    }
}

void MeshRenderer::render() const
{
    ensureIntegrity();

    for (SubMeshIndex i = 0; i < m_mesh->m_subMeshes.size(); ++i)
    {
        m_materials[i]->use();
        bindAndRender(i);
    }
}

void MeshRenderer::render(Shader* shader)
{
    ensureIntegrity();

    for (SubMeshIndex i = 0; i < m_mesh->m_subMeshes.size(); ++i)
    {
        m_materials[i]->use(shader);
        bindAndRender(i);
    }
}

void MeshRenderer::setMesh(std::shared_ptr<Mesh> mesh)
{
    m_mesh = mesh;
}

void MeshRenderer::addMaterial(std::shared_ptr<Material> material)
{
    m_materials.push_back(material);
}

void MeshRenderer::setMaterial(std::shared_ptr<Material> material, uint8_t index)
{
    assert(index < m_materials.size());
    m_materials[index] = material;
}

void MeshRenderer::setModelMatrix(const glm::mat4& matrix)
{
    for (auto m : m_materials)
    {
        m->setMatrix("u_model", matrix);
        m->setMatrix("u_modelIT", glm::transpose(glm::inverse(matrix)));
    }
}

void MeshRenderer::performForAllMaterials(std::function<void(Material*)> func)
{
    for (auto m : m_materials)
        func(m.get());
}

void MeshRenderer::bindAndRender(SubMeshIndex subMeshIdx) const
{
    assert(m_mesh);
    assert(subMeshIdx < m_mesh->m_subMeshes.size());

    Mesh::SubMesh& subMesh = m_mesh->m_subMeshes[subMeshIdx];
    Mesh::SubMeshRenderData& renderData = m_mesh->m_subMeshRenderData[subMeshIdx];

    assert(renderData.vbo != 0 && renderData.vao != 0);

    glBindVertexArray(renderData.vao);

    if (subMesh.indices.size() > 0)
    {
        assert(renderData.ibo != 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData.ibo);
    }

    if (subMesh.indices.size() > 0)
        glDrawElements(renderData.renderMode, GLsizei(subMesh.indices.size()), GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(renderData.renderMode, 0, GLsizei(subMesh.vertices.size()));
}

void MeshRenderer::ensureIntegrity() const
{
    assert(m_mesh);
    assert(m_materials.size() == m_mesh->m_subMeshes.size());

    for (auto m : m_materials)
    {
        assert(m);
    }
}
