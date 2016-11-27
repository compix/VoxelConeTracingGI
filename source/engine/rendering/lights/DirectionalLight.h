#pragma once
#include <engine/ecs/ECS.h>
#include <glm/glm.hpp>
#include <GL/glew.h>

class DirectionalLight : public Component
{
public:
    DirectionalLight() { }

    DirectionalLight(const glm::vec3& color, float intensity)
        : color(color), intensity(intensity) { }

    void onShowInEditor() override;

    std::string getName() const override { return "Directional Light"; }

    glm::vec3 color{1.0f};
    float intensity{1.0f};

    bool shadowsEnabled{ true };
    GLuint shadowMap{0};

    glm::mat4 view;
    glm::mat4 proj;
    float zNear{0.3f};
    float zFar{30.0f};
    float pcfRadius{ 0.001f };
};
