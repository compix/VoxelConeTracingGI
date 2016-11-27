#pragma once
#include <unordered_map>
#include "GUITexture.h"

class GUI
{
public:
    static void showHint(const std::string& desc);

public:
    static std::unordered_map<GLuint, GUITexture> textures;
};
