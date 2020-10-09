#ifndef PONG_VK_COMPONENTS_H
#define PONG_VK_COMPONENTS_H

#include <glm/glm.hpp>

namespace Pong {

    enum class CollisionDirection {
        RIGHT   = 0,
        UP      = 1,
        LEFT    = 2,
        DOWN    = 3,
        DIAGONAL_UP_RIGHT = 4, 
        DIAGONAL_UP_LEFT = 5,
        DIAGONAL_DOWN_RIGHT = 6,
        DIAGONAL_DOWN_LEFT = 7
    };

    struct Transform {
        glm::vec2 position { 0.0f, 0.0f };
        glm::vec3 rotation { 0.0f, 0.0f, 0.0f };
        glm::vec3 scale { 0.0f, 0.0f, 0.0f };
        float rotationAngle {0.f};
    };

    struct Velocity {
        glm::vec2 positionVelocity { 0.0f, 0.0f };
        glm::vec3 rotationVelocity { 0.0f, 0.0f, 0.0f };
    };

    struct CollisionInfo {
        CollisionDirection direction {CollisionDirection::RIGHT};
        glm::vec2 difference {0.0f, 0.0f};
    };

    struct RectBounds {
        float minX {0};
        float minY {0};
        float maxX {0};
        float maxY {0};
    };

    void addVelocity(Transform&, Velocity&);
    bool isOverlapping(RectBounds&, RectBounds&);
    CollisionInfo resolveCollision(Transform&, Transform&, RectBounds&, RectBounds&, glm::vec2&);
    RectBounds initialiseRectBounds(Transform&);
    void updateRectBounds(RectBounds&, Transform&);
}

#endif //PONG_VK_COMPONENTS_H
