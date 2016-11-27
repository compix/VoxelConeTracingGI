#include "Transform.h"
#include <engine/util/math.h>
#include <imgui/imgui.h>

Transform::Transform(const BBox& bbox)
{
    setBBox(bbox);
    m_lastFrameWorldBBox = m_worldBBox;
}

void Transform::onShowInEditor()
{
    m_eulerAnglesWorld = math::toDegrees(m_eulerAnglesWorld);

    bool positionChanged = ImGui::DragFloat3("Position", &m_position[0], 0.05f);
    bool scaleChanged = ImGui::DragFloat3("Scale", &m_scale[0], 0.05f);
    bool rotationChanged = ImGui::DragFloat3("Rotation", &m_eulerAnglesWorld[0], 0.5f, -360.0f, 360.0f);

    m_eulerAnglesWorld = math::toRadians(m_eulerAnglesWorld);

    if (positionChanged || scaleChanged || rotationChanged)
    {
        // This will also update the cache for position and scale
        setEulerAngles(m_eulerAnglesWorld);
    }
}

void Transform::lateUpdate()
{
    m_changedSinceLastFrame = false;
    m_lastFrameWorldBBox = m_worldBBox;
}

glm::mat3 Transform::getLocalToWorldRotationMatrix() const
{
    return glm::toMat3(getLocalToWorldRotation());
}

void Transform::setLocalPosition(const glm::vec3& position)
{
    m_position = position;
    updateCache();
}

void Transform::setLocalRotation(const glm::quat& rotation)
{
    m_rotation = rotation;
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}

void Transform::setPosition(const glm::vec3& position)
{
    m_position = m_parent ? m_parent->transformPointToLocal(position) : position;
    updateCache();
}

void Transform::setRotation(const glm::quat& rotation)
{
    m_rotation = m_parent ? m_parent->getWorldToLocalRotation() * rotation : rotation;
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}

glm::vec3 Transform::getApproximateScale() const
{
    const glm::mat4& world = getLocalToWorldMatrix();

    return glm::vec3(glm::length(world[0]), glm::length(world[1]), glm::length(world[2]));
}

void Transform::setLocalScale(const glm::vec3& scale)
{
    m_scale = scale;
    updateCache();
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& worldUp)
{
    glm::mat3 rotation;

    rotation[2] = glm::normalize(target - getPosition());
    rotation[0] = glm::normalize(glm::cross(worldUp, rotation[2]));
    rotation[1] = glm::cross(rotation[2], rotation[0]);

    setRotation(glm::toQuat(rotation));
}

void Transform::setLocalEulerAngles(const glm::vec3& eulerAngles)
{
    m_rotation = math::eulerYXZQuat(eulerAngles);
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}

glm::vec3 Transform::getLocalEulerAngles() const { return math::eulerAngles(m_rotation); }

void Transform::setEulerAngles(const glm::vec3& eulerAngles)
{
    glm::quat rotation = math::eulerYXZQuat(eulerAngles);
    m_rotation = m_parent ? m_parent->getWorldToLocalRotation() * rotation : rotation;
    updateCache();
    m_eulerAnglesWorld = eulerAngles;
}

glm::vec3 Transform::getEulerAngles() const
{
    return math::eulerAngles(getLocalToWorldRotation());
}

void Transform::setParent(ComponentPtr<Transform> parent)
{
    if (m_parent == parent)
        return;

    if (m_parent)
        m_parent->m_children.erase(std::remove(m_children.begin(), m_children.end(), ComponentPtr<Transform>(getOwner())));

    m_parent = parent;
    m_root = parent->getRoot();
    m_parent->m_children.push_back(ComponentPtr<Transform>(getOwner()));

    updateCacheHierarchy();
}

glm::vec3 Transform::transformPointToWorld(const glm::vec3& point) const
{
    return glm::vec3(getLocalToWorldMatrix() * glm::vec4(point, 1.0f));
}

glm::vec3 Transform::transformVectorToWorld(const glm::vec3& vector) const
{
    return glm::vec3(getLocalToWorldMatrix() * glm::vec4(vector, 0.0f));
}

glm::vec3 Transform::transformPointToLocal(const glm::vec3& point) const
{
    return glm::vec3(getWorldToLocalMatrix() * glm::vec4(point, 1.0f));
}

glm::vec3 Transform::transformVectorToLocal(const glm::vec3& vector) const
{
    return glm::vec3(getWorldToLocalMatrix() * glm::vec4(vector, 0.0f));
}

void Transform::setBBox(const BBox& bbox) noexcept
{
    m_originalBBox = bbox;
    m_worldBBox = bbox.toWorld(m_localToWorldMatrix);
    m_changedSinceLastFrame = true;
    m_lastFrameWorldBBox = m_worldBBox;
}

void Transform::updateCache()
{
    m_localMatrix = glm::translate(m_position) * glm::toMat4(m_rotation) * glm::scale(m_scale);
    m_localMatrixInv = glm::inverse(m_localMatrix);

    updateCacheHierarchy();
}

void Transform::updateCacheHierarchy()
{
    if (m_parent)
    {
        m_localToWorldMatrix = m_parent->getLocalToWorldMatrix() * m_localMatrix;
        m_worldToLocalMatrix = m_localMatrixInv * m_parent->getWorldToLocalMatrix();
        m_localToWorldRotation = m_parent->getLocalToWorldRotation() * m_rotation;
        m_worldToLocalRotation = glm::inverse(m_localToWorldRotation);
    }
    else
    {
        m_localToWorldMatrix = m_localMatrix;
        m_worldToLocalMatrix = m_localMatrixInv;
        m_localToWorldRotation = m_rotation;
        m_worldToLocalRotation = glm::inverse(m_localToWorldRotation);
    }

    m_changedSinceLastFrame = true;
    m_worldBBox = m_originalBBox.toWorld(m_localToWorldMatrix);

    // Update all children
    for (auto& child : m_children)
        child->updateCacheHierarchy();
}

glm::mat4 Transform::getLocalMatrix() const
{
    return m_localMatrix;
}

glm::mat4 Transform::getLocalInverseMatrix() const
{
    return m_localMatrixInv;
}

void Transform::pitch(float angle) noexcept
{
    // Rotate up and look vector about the right vector.
    glm::mat3 rotation;

    rotation[0] = getRight();
    rotation[1] = glm::vec3(glm::rotate(angle, getRight()) * glm::vec4(getUp(), 0));
    rotation[2] = glm::vec3(glm::rotate(angle, getRight()) * glm::vec4(getForward(), 0));

    setRotation(glm::normalize(glm::toQuat(rotation)));
}

void Transform::rotateY(float angle) noexcept
{
    // Rotate the basis vectors about the world y-axis.
    glm::mat3 rotation;
    rotation[0] = glm::rotateY(getRight(), angle);
    rotation[1] = glm::rotateY(getUp(), angle);
    rotation[2] = glm::rotateY(getForward(), angle);

    setRotation(glm::normalize(glm::toQuat(rotation)));
}

void Transform::roll(float angle) noexcept
{
    // Rotate up and right vector about the forward vector.
    glm::mat3 rotation;

    rotation[0] = glm::vec3(glm::rotate(angle, getForward()) * glm::vec4(getRight(), 0));
    rotation[1] = glm::vec3(glm::rotate(angle, getForward()) * glm::vec4(getUp(), 0));
    rotation[2] = getForward();

    setRotation(glm::normalize(glm::toQuat(rotation)));
}
