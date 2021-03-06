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

    CollisionInfo resolveCollision(Transform& transformA, Transform& transformB,
        RectBounds& rectA, RectBounds& rectB, glm::vec2& direction) {

        glm::vec2 directions[] = {
            {1.0f, 0.0f},   // right
            {0.0f, 1.0f},   // up
            {-1.0f, 0.0f},  // left
            {0.0, -1.0},    // down
            {1.0, 1.0},     // diagonal up-right
            {-1.0, 1.0},    // diaglonal up-left
            {1.0, -1.0},    // diagonal down-right
            {-1.0, -1.0},   // diagonal down-left
        };

        float max = 0.0f;
        size_t bestMatch = -1;
        for (size_t i = 0; i < 8; i++) {
            float dot_product = glm::dot(glm::normalize(direction), directions[i]);
            if (dot_product > max) {
                bestMatch = i;
                max = dot_product;
            }
        }

        glm::vec2 hit_direction = directions[bestMatch];

        float dirX = glm::sign(hit_direction.x);
        float dirY = glm::sign(hit_direction.y);

        glm::vec2 difference = { 0.0f, 0.0f};

        float distanceFromPoints {0.0f};

        if (dirX == 1 || dirX == -1) {
            if (transformA.position.x >= transformB.position.x) {
                distanceFromPoints = rectB.maxX - rectA.minX;
            } else {
                distanceFromPoints = rectB.minX - rectA.maxX;
            }
            difference.x = distanceFromPoints;
        }
        if (dirY == 1 || dirY == -1) {
            if (transformA.position.y >= transformB.position.y) {
                distanceFromPoints = rectB.maxY - rectA.minY;
            } else {
                distanceFromPoints = rectB.minY - rectA.maxY;
            }
            difference.y = distanceFromPoints;
        }

        return { (CollisionDirection)bestMatch, difference };
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

