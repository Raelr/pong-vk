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

    glm::vec2 resolveCollision(Transform& transformA, Transform& transformB,
        RectBounds& rectB, glm::vec2& direction) {

        glm::vec2 directions[] = {
            {1.0f, 0.0f},
            {0.0f, 1.0f},
            {-1.0f, 0.0f},
            {0.0, -1.0}
        };

        float max = 0.0f;
        size_t best_match = -1;
        for (size_t i = 0; i < 4; i++) {
            float dot_product = glm::dot(glm::normalize(direction), directions[i]);
            if (dot_product > max) {
                best_match = i;
                max = dot_product;
            }
        }

        glm::vec2 hit_direction = directions[best_match];

        float dirX = glm::sign(direction.x);
        float dirY = glm::sign(direction.y);

        glm::vec2 difference = { 0.0f, 0.0f};

        float timeXCollision, timeYCollision {0.0f};

        // Right of Left collisions
        if ((dirX == 1 && dirY == 0) || (dirX == -1 && dirY == 0))  {
            if (dirX == 1) {
                timeXCollision = ((transformA.position.x + (transformA.scale.x * 0.5)) - rectB.minX) / -600.0f;
            } else {
                timeXCollision = ((transformA.position.x - (transformA.scale.x * 0.5)) - rectB.minX) / 600.0f;
            }
            PONG_INFO("X collision time: {0}", timeXCollision * 600.0f);
            difference.x = timeXCollision * 600.0f;
            direction.x = -direction.x;
        }

        // Top and Bottom collisions
        if ((dirY == 1 && dirX == 0) || (dirY == -1 && dirX == 0))  {
            if (dirY == 1) {
                timeYCollision = (rectB.minY - (transformA.position.y + (transformA.scale.y * 0.5))) / 600.0f;
            } else {
                timeYCollision = (rectB.maxY - (transformA.position.y - (transformA.scale.y * 0.5))) / 600.0f;
            }
            difference.y = timeYCollision * 600.0f;
            direction.y = -direction.y;
        }

        // Diagonals
        if ((dirY == 1 && dirX == 1) || (dirY == -1 && dirX == -1) || (dirY == -1 && dirX == 1) || (dirY == 1 && dirX == -1) )  {
            if (dirX == 1) {
                timeXCollision = (rectB.minX - (transformA.position.x + (transformA.scale.x * 0.5))) / 600.0f;
            } else {
                timeXCollision = (rectB.maxX - (transformA.position.x - (transformA.scale.x * 0.5))) / 600.0f;
            }

            if (dirY == 1) {
                timeYCollision = (rectB.minY - (transformA.position.y + (transformA.scale.y * 0.5))) / 600.0f;
            } else {
                timeYCollision = (rectB.maxY - (transformA.position.y - (transformA.scale.y * 0.5))) / 600.0f;
            }
            PONG_INFO("Y collision time: {0}", timeYCollision);
            PONG_INFO("X collision time: {0}", timeXCollision);

            difference = (timeXCollision < timeYCollision) ? glm::vec2(timeXCollision, 0.0f)
                    : glm::vec2(0.0f, timeYCollision);

            direction = -direction;
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

