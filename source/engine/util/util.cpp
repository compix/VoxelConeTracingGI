#include "util.h"
#include <engine/rendering/geometry/Mesh.h>
#include <iterator>
#include <cstddef>

BBox util::computeBBox(const Mesh& mesh)
{
    BBox box;

    for (auto& subMesh : mesh.getSubMeshes())
        for (auto& v : subMesh.vertices)
            box.unite(v);

    return box;
}

std::vector<std::string> util::split(const std::string& s, const std::string& delimiter)
{
    std::vector<std::string> strings;
    std::size_t last = 0;
    std::size_t next = 0;
    while ((next = s.find(delimiter, last)) != std::string::npos)
    {
        std::string sub = s.substr(last, next - last);
        if (sub.length() > 0)
            strings.push_back(sub);
        last = next + delimiter.length();
    }

    std::string sub = s.substr(last, next - last);
    if (sub.length() > 0)
        strings.push_back(sub);

    return strings;
}

std::vector<std::string> util::split(const std::string& s, char delimiter)
{
    std::vector<std::string> strings;
    std::string w;
    std::stringstream stream(s);

    while (getline(stream, w, delimiter))
        strings.push_back(w);

    return strings;
}

std::vector<std::string> util::splitWhitespace(const std::string& s)
{
    std::istringstream buffer(s);
    std::vector<std::string> strings((std::istream_iterator<std::string>(buffer)), std::istream_iterator<std::string>());
    return strings;
}
