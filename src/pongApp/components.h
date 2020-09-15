#ifndef PONG_VK_COMPONENTS_H
#define PONG_VK_COMPONENTS_H

#include <glm/glm.hpp>

namespace Pong {

    struct Transform {
        glm::vec3 position { 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation { 0.0f, 0.0f, 0.0f };
        glm::vec3 scale { 0.0f, 0.0f, 0.0f };
        float rotationAngle {0.f};
    };

    struct Velocity {
        glm::vec3 positionVelocity { 0.0f, 0.0f, 0.0f };
        glm::vec3 rotationVelocity { 0.0f, 0.0f, 0.0f };
    };

    struct RectAABB {
        float minX, minY, maxX, maxY {0.0f};
    };

    void addVelocity(Transform&, Velocity&);
    RectAABB initialiseAABBCollision(Transform&);
    bool isOverlapping(Transform& transformA, Transform& transformB);
}

#endif //PONG_VK_COMPONENTS_H
