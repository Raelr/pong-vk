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

    struct RectBounds {
        float minX {0};
        float minY {0};
        float maxX {0};
        float maxY {0};
    };

    void addVelocity(Transform&, Velocity&);
    bool isOverlapping(RectBounds&, RectBounds&);
    glm::vec3 resolveCollision(RectBounds& , RectBounds&, glm::vec2);
    RectBounds initialiseRectBounds(Transform&);
    void updateRectBounds(RectBounds&, Transform&);
}

#endif //PONG_VK_COMPONENTS_H
