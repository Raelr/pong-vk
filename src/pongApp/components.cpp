#include "components.h"
#include "../logger.h"

namespace Pong {

    void addVelocity(Transform& transform, Velocity& velocity) {
        transform.position += velocity.positionVelocity;
    }

    bool isOverlapping(RectBounds& rectA, RectBounds& rectB) {

        bool collisionX = (rectB.maxX > rectA.maxX && rectB.minX < rectA.maxX)
                || (rectB.maxX > rectA.minX && rectB.minX < rectA.minX);

        bool collisionY = (rectB.maxY > rectA.maxY && rectB.minY < rectA.maxY)
                || (rectB.maxY > rectA.minY && rectB.minY < rectA.minY);

        return collisionX && collisionY;
    }

    glm::vec3 resolveCollision(Transform& transformA, RectBounds& rectA, Transform& transformB,
        RectBounds& rectB, glm::vec2 direction) {

        float dirX = glm::sign(direction.x);
        float dirY = glm::sign(direction.y);

        glm::vec3 difference = {0.0f, 0.0f, 0.0f};

        if (dirX == 1) {
            difference.x = (rectB.minX - transformA.position.x) - (transformA.scale.x * 0.5f);
        } else if (dirX == -1) {
            difference.x = (rectB.maxX - transformA.position.x) + (transformA.scale.x * 0.5f);
        }

//        if (dirY == 1) {
//            difference.y = (rectB.minY - transformA.position.y) - (transformA.scale.y * 0.5f);
//            PONG_INFO("UP");
//        } else if (dirY == -1) {
//            difference.y = (rectB.maxY - transformA.position.y) + (transformA.scale.x * 0.5f);
//        }

        PONG_INFO("DIFFERENCE (X: {0} | Y: {1})", difference.x, difference.y);

        return difference;
    }

    RectBounds initialiseRectBounds(Transform& transform) {
        RectBounds rect{};
        updateRectBounds(rect, transform);
        return rect;
    }

    void updateRectBounds(RectBounds& rect, Transform& transform) {
        rect.minX = transform.position.x - (transform.scale.x / 2.0f);
        rect.minY = transform.position.y - (transform.scale.y / 2.0f);
        rect.maxX = transform.position.x + (transform.scale.x / 2.0f);
        rect.maxY = transform.position.y + (transform.scale.y / 2.0f);
    }
}

