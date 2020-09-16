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

    void addVelocity(Transform&, Velocity&);
    bool isOverlapping(Transform& transformA, Transform& transformB);
    void resolveCollision(Transform& transformA, Transform& transformB, glm::vec2 direction);
}

#endif //PONG_VK_COMPONENTS_H
