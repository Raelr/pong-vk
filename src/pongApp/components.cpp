#include "components.h"

namespace Pong {

    void addVelocity(Transform& transform, Velocity& velocity) {
        transform.position += velocity.positionVelocity;
        velocity.positionVelocity = glm::vec3(0.0f);
    }

    RectAABB initialiseAABBCollision(Transform& transform) {
        RectAABB rectCollider {
        transform.position.x - (transform.scale.x / 2.0f),
        transform.position.y - (transform.scale.y / 2.0f),
        transform.position.x + (transform.scale.x / 2.0f),
        transform.position.y + (transform.scale.y / 2.0f)
        };

        return rectCollider;
    }

    bool isOverlapping(Transform& transformA, Transform& transformB) {

        bool collisionX = ((transformA.position.x + (transformA.scale.x * 0.5)) >= transformB.position.x
            && (transformB.position.x + (transformB.scale.x * 0.5)) >= transformA.position.x);

        bool collisionY = ((transformA.position.y + (transformA.scale.y * 0.5)) >= transformB.position.y &&
                (transformB.position.y + (transformA.scale.y * 0.5)) >= transformA.position.y);

        return collisionX && collisionY;
    }

    void resolveCollision(Transform& transformA, Transform& transformB, glm::vec2 direction) {

        if (glm::sign(direction.x) == 1) {
            transformA.position.x = (transformB.position.x - (transformB.scale.x));
        } else if (glm::sign(direction.x) == -1) {
            transformA.position.x = (transformB.position.x + (transformB.scale.x));
        }
    }
}

