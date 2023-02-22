#include "Model.h"
#include <cstddef>

void Model::addChild(std::shared_ptr<Model> model)
{
    children.push_back(model);
    model->parent = this;
}

std::vector<Mesh::SubMesh> Model::getAllSubMeshes() const
{
    std::vector<Mesh::SubMesh> allMeshes;
    allMeshes.insert(allMeshes.end(), subMeshes.begin(), subMeshes.end());

    for (auto m : children)
    {
        auto childMeshes = m->getAllSubMeshes();
        allMeshes.insert(allMeshes.end(), childMeshes.begin(), childMeshes.end());
    }

    return allMeshes;
}

std::vector<MaterialDescription> Model::getAllMaterials() const
{
    std::vector<MaterialDescription> allMaterials;
    allMaterials.insert(allMaterials.end(), materials.begin(), materials.end());

    for (auto m : children)
    {
        auto childMaterials = m->getAllMaterials();
        allMaterials.insert(allMaterials.end(), childMaterials.begin(), childMaterials.end());
    }

    return allMaterials;
}

std::size_t Model::getTriangleCount() const
{
    std::size_t triangleCount = 0;

    for (auto& sm : subMeshes)
        triangleCount += sm.indices.size() / 3;

    for (auto& c : children)
        triangleCount += c->getTriangleCount();

    return triangleCount;
}
