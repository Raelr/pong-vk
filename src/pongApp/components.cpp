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

    glm::vec3 resolveCollision(RectBounds& RectBoundsA, RectBounds& RectBoundsB, glm::vec2 direction) {

        float xDir = glm::sign(direction.x);
        float yDir = glm::sign(direction.y);

        glm::vec3 difference = {0.0f, 0.0f, 0.0f};

        if (xDir == 1) {
            difference.x = RectBoundsB.minX - RectBoundsA.maxX;
        }
        else if (xDir == -1) {
            difference.x = RectBoundsB.maxX - RectBoundsA.minX;
        }
        if (yDir == 1) {
            difference.y = RectBoundsB.maxY - RectBoundsA.minY;
        }
        else if (yDir == -1) {
            difference.y = RectBoundsB.minY - RectBoundsA.maxY;
        }

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

