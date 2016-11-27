#include "serialization.h"
#include "Model.h"

// ****************************** Model ******************************
void Serializer::write(std::ofstream& os, const Model& val)
{
    write(os, val.position);
    write(os, val.scale);
    write(os, val.rotation);

    // Number of SubMeshes
    write(os, val.subMeshes.size());

    // SubMehes
    for (auto& sm : val.subMeshes)
    {
        writeVector(os, sm.indices);
        writeVector(os, sm.vertices);
        writeVector(os, sm.normals);
        writeVector(os, sm.tangents);
        writeVector(os, sm.uvs);
        writeVector(os, sm.colors);
    }

    // Number of children
    write(os, val.children.size());

    // Children
    for (auto& child : val.children) { write(os, *child); }
}

std::shared_ptr<Model> Serializer::readModel(std::ifstream& is)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();

    model->position = read<glm::vec3>(is);
    model->scale = read<glm::vec3>(is);
    model->rotation = read<glm::quat>(is);

    // Number of SubMeshes
    model->subMeshes.resize(read<size_t>(is));

    // SubMeshes
    for (size_t i = 0; i < model->subMeshes.size(); ++i)
    {
        Mesh::SubMesh& subMesh = model->subMeshes[i];

        readVector(is, subMesh.indices);
        readVector(is, subMesh.vertices);
        readVector(is, subMesh.normals);
        readVector(is, subMesh.tangents);
        readVector(is, subMesh.uvs);
        readVector(is, subMesh.colors);
    }

    // Number of children
    model->children.resize(read<size_t>(is));

    // Children
    for (size_t i = 0; i < model->children.size(); ++i)
    {
        model->children[i] = readModel(is);
        model->children[i]->parent = model.get();
    }

    return model;
}
