#include "components.h"
#include "../logger.h"

namespace Pong {

    void addVelocity(Transform& transform, Velocity& velocity) {
        transform.position += velocity.positionVelocity;
        velocity.positionVelocity = glm::vec3(0.0f);
    }

    bool isOverlapping(Transform& transformA, Transform& transformB) {

        float maxXA = transformA.position.x + (transformA.scale.x / 2.0f);
        float minXA = transformA.position.x - (transformA.scale.x / 2.0f);
        float maxXB = transformB.position.x + (transformB.scale.x / 2.0f);
        float minXB = transformB.position.x - (transformB.scale.x / 2.0f);

        float maxYA = transformA.position.y + (transformA.scale.y / 2.0f);
        float minYA = transformA.position.y - (transformA.scale.y / 2.0f);
        float maxYB = transformB.position.y + (transformB.scale.y / 2.0f);
        float minYB = transformB.position.y - (transformB.scale.y / 2.0f);

        bool collisionX = (maxXB > maxXA && minXB < maxXA) || (maxXB > minXA && minXB < minXA);

        bool collisionY = (maxYB > maxYA && minYB < maxYA) || (maxYB > minYA && minYB < minYA);

        return collisionX && collisionY;
    }

    void resolveCollision(Transform& transformA, Transform& transformB, glm::vec2 direction) {

        float rightMostA = transformA.position.x + (transformA.scale.x / 2.0f);
        float leftmostB = transformB.position.x - (transformB.scale.x / 2.0f);
        float rightMostB = transformB.position.x + (transformB.scale.x / 2.0f);
        float leftmostA = transformA.position.x - (transformA.scale.x / 2.0f);

        float topMostA = transformA.position.y + (transformA.scale.y / 2.0f);
        float bottomMostB = transformB.position.y - (transformB.scale.y / 2.0f);
        float topMostB = transformB.position.y + (transformB.scale.y / 2.0f);
        float bottomMostA = transformA.position.y - (transformA.scale.y / 2.0f);

        float xDir = glm::sign(direction.x);
        float yDir = glm::sign(direction.y);

        if (xDir == 1) {
            float difference = rightMostA - leftmostB;
            transformA.position.x -= difference;
        }
        else if (xDir == -1) {
            float difference = rightMostB - leftmostA;
            transformA.position.x += difference;
        }
        if (yDir == 1) {
            float difference = topMostA - bottomMostB;
            transformA.position.x -= difference;
        }
        else if (yDir == -1) {
            float difference = bottomMostA - topMostB;
            transformA.position.x += difference;
        }
    }

    RectBounds initialiseRectBounds(Transform& transform) {
        return {
        transform.position.x - (transform.scale.x / 2.0f),
        transform.position.y - (transform.scale.y / 2.0f),
        transform.position.x + (transform.scale.x / 2.0f),
        transform.position.y + (transform.scale.y / 2.0f)
        };
    }

    void addVelocity(RectBounds& bounds, Velocity& velocity) {
        bounds.minX += velocity.positionVelocity.x;
        bounds.maxX += velocity.positionVelocity.x;
        bounds.minY += velocity.positionVelocity.y;
        bounds.maxY += velocity.positionVelocity.y;
    }
}

