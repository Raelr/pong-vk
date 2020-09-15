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

        bool collisionX = ((transformA.position.x + transformA.scale.x) >= transformB.position.x
            && (transformB.position.x + transformB.scale.x) >= transformA.position.x);

        bool collisionY = ((transformA.position.y + transformA.scale.y) >= transformB.position.y &&
                (transformB.position.y + transformB.scale.y) >= transformA.position.y);

        return collisionX && collisionY;
    }
}

