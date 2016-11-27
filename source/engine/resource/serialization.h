#pragma once
#include <fstream>
#include <memory>
#include <engine/rendering/geometry/Mesh.h>

struct Model;
class Transform;

class Serializer
{
public:
    // Generic
    template <class T>
    static void write(std::ofstream& os, const T& val) { os.write(reinterpret_cast<const char*>(&val), sizeof(T)); }

    template <class T>
    static void writeVector(std::ofstream& os, const std::vector<T>& v);

    template <class T>
    static void read(std::ifstream& is, T& val) { is.read(reinterpret_cast<char*>(&val), sizeof(T)); }

    template <class T>
    static T read(std::ifstream& is)
    {
        T val;
        is.read(reinterpret_cast<char*>(&val), sizeof(T));
        return val;
    }

    template <class T>
    static void readVector(std::ifstream& is, std::vector<T>& v);

    // Model
    static void write(std::ofstream& os, const Model& val);
    static std::shared_ptr<Model> readModel(std::ifstream& is);
};

template <class T>
void Serializer::writeVector(std::ofstream& os, const std::vector<T>& v)
{
    write(os, v.size());
    for (auto& val : v)
        write(os, val);
}

template <class T>
void Serializer::readVector(std::ifstream& is, std::vector<T>& v)
{
    v.resize(read<size_t>(is));
    for (size_t i = 0; i < v.size(); ++i)
        v[i] = read<T>(is);
}
