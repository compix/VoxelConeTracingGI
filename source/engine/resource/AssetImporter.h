#pragma once
#include <memory>
#include <string>
#include "Model.h"

struct aiScene;
struct aiNode;
class Mesh;

class AssetImporter
{
public:
    static std::shared_ptr<Model> import(const std::string& filename);

private:
    static std::shared_ptr<Model> process(const aiScene* scene, const aiNode* aiNode);
};
