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

        if (glm::sign(direction.x) == 1) {
            float leftXB = transformB.position.x - (transformB.scale.x / 2.0f);

            transformA.position.x = leftXB + 10;
        } else if (glm::sign(direction.x) == -1) {
        }
    }
}

