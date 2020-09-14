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

    bool isOverlapping(RectAABB&, RectAABB&) {
        bool isOverlapping = false;
        return true;
    }
}

