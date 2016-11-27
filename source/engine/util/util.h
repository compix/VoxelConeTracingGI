#pragma once
#include <engine/geometry/BBox.h>
#include <vector>

class Mesh;

namespace util
{
    /**
     *  Usage: offset(&SomeStructOrClass::someMember)
     */
    template <class TClass, class TMember>
    const void* offset(const TMember TClass::* member)
    {
        return reinterpret_cast<const void*>(&(reinterpret_cast<TClass*>(0) ->* member));
    }

    template <class T>
    class TD;

    BBox computeBBox(const Mesh& mesh);

    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
    std::vector<std::string> split(const std::string& s, char delimiter);
    std::vector<std::string> splitWhitespace(const std::string& s);
}
