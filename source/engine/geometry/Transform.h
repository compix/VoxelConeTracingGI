#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <engine/ecs/EntityManager.h>
#include "BBox.h"

class Transform : public Component
{
public:
    Transform() { }

    explicit Transform(const BBox& bbox);

    void onShowInEditor() override;

    std::string getName() const override { return "Transform"; }

    void lateUpdate() override;

    /**
    * Returns the transformation matrix from the local coordinate system of this transform to the world coordinate system.
    */
    const glm::mat4& getLocalToWorldMatrix() const { return m_localToWorldMatrix; }

    /**
    * Returns the transformation matrix from the world coordinate system to the local coordinate system of this transform.
    */
    const glm::mat4& getWorldToLocalMatrix() const { return m_worldToLocalMatrix; }

    glm::vec3 getRight() const { return getLocalToWorldRotationMatrix()[0]; }

    glm::vec3 getUp() const { return getLocalToWorldRotationMatrix()[1]; }

    glm::vec3 getForward() const { return getLocalToWorldRotationMatrix()[2]; }

    /**
    * Sets the position in world space.
    */
    void setPosition(const glm::vec3& position);

    /**
    * Returns the position in world space.
    */
    glm::vec3 getPosition() const { return glm::vec3(getLocalToWorldMatrix()[3]); }

    /**
    * Sets the rotation in world space.
    */
    void setRotation(const glm::quat& rotation);

    /**
    * Returns the rotation quaternion in world space.
    */
    const glm::quat& getRotation() const { return m_localToWorldRotation; }

    /**
    * Returns the scale in world space. Due to skewing the 3 component scale
    * is only an approximation. An accurate scale is returned if no skew is applied.
    */
    glm::vec3 getApproximateScale() const;

    void setLocalPosition(const glm::vec3& position);
    void setLocalRotation(const glm::quat& rotation);
    void setLocalScale(const glm::vec3& scale);

    const glm::vec3& getLocalScale() const { return m_scale; }

    const glm::vec3& getLocalPosition() const { return m_position; }

    const glm::quat& getLocalRotation() const { return m_rotation; }

    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
    * Set the euler angles in local space.
    * The given angles correspond to a rotation in radians about the matching axis.
    * Rotation order: YXZ
    */
    void setLocalEulerAngles(const glm::vec3& eulerAngles);
    glm::vec3 getLocalEulerAngles() const;

    /**
    * Set the euler angles in world space.
    * The given angles correspond to a rotation in radians about the matching axis.
    * Rotation order: YXZ
    */
    void setEulerAngles(const glm::vec3& eulerAngles);

    /**
    * Returns the rotation as euler angles in world space.
    */
    glm::vec3 getEulerAngles() const;

    void setParent(ComponentPtr<Transform> parent);

    /**
    * Transforms local point to world space.
    */
    glm::vec3 transformPointToWorld(const glm::vec3& point) const;

    /**
    * Transforms local vector to world space.
    */
    glm::vec3 transformVectorToWorld(const glm::vec3& vector) const;

    /**
    * Transforms world point to local space.
    */
    glm::vec3 transformPointToLocal(const glm::vec3& point) const;

    /**
    * Transforms world vector to local space.
    */
    glm::vec3 transformVectorToLocal(const glm::vec3& vector) const;

    ComponentPtr<Transform> getRoot() const { return m_root ? m_root : ComponentPtr<Transform>(getOwner()); }

    bool hasChildren() const { return m_children.size() > 0; }

    const std::vector<ComponentPtr<Transform>>& getChildren() const { return m_children; }

    void pitch(float angle) noexcept;
    void rotateY(float angle) noexcept;
    void roll(float angle) noexcept;

    void strafe(float d) noexcept { setPosition(getPosition() + getRight() * d); }

    void walk(float d) noexcept { setPosition(getPosition() + getForward() * d); }

    void move(const glm::vec3& posDelta) { setPosition(getPosition() + posDelta); }

    bool hasChangedSinceLastFrame() const { return (m_changedSinceLastFrame & 1) > 0; }

    void setBBox(const BBox& bbox) noexcept;

    const BBox& getBBox() const noexcept { return m_worldBBox; }

    const BBox& getLastFrameBBox() const noexcept { return m_lastFrameWorldBBox; }

private:
    void updateCache();
    void updateCacheHierarchy();

    glm::mat4 getLocalMatrix() const;
    glm::mat4 getLocalInverseMatrix() const;

    glm::mat3 getLocalToWorldRotationMatrix() const;

    const glm::quat& getLocalToWorldRotation() const { return m_localToWorldRotation; }

    const glm::quat& getWorldToLocalRotation() const { return m_worldToLocalRotation; }

private:
    ComponentPtr<Transform> m_root;
    ComponentPtr<Transform> m_parent;
    std::vector<ComponentPtr<Transform>> m_children;

    // Local coordinate attributes
    glm::vec3 m_position;
    glm::vec3 m_scale{1.0f,1.0f,1.0f};
    glm::quat m_rotation;

    // Cache for performance
    glm::mat4 m_localMatrix;
    glm::mat4 m_localMatrixInv;
    glm::mat4 m_localToWorldMatrix;
    glm::mat4 m_worldToLocalMatrix;
    glm::quat m_localToWorldRotation;
    glm::quat m_worldToLocalRotation;

    // Editor specific members
    glm::vec3 m_eulerAnglesWorld; // Euler angles in world space

    bool m_changedSinceLastFrame{false};

    BBox m_originalBBox;
    BBox m_worldBBox;
    BBox m_lastFrameWorldBBox;
};
