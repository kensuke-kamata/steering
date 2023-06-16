#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Component.h"

namespace steering {
/**
 * Transform Matrix (2D)
 */
inline glm::mat3 TransformMatrix(
    const glm::vec2 &position,
    const glm::vec2 &heading,
    const glm::vec2 &scale)
{
    glm::mat3 transform(1.0f);

    // Apply scale
    transform[0][0] = scale.x;
    transform[1][1] = scale.y;

    // Apply rotation
    float rotation = std::atan2(heading.y, heading.x);
    float c = std::cos(rotation);
    float s = std::sin(rotation);
    glm::mat3 rotate(
        c, -s,  0,
        s,  c,  0,
        0,  0,  1);
    transform = rotate * transform;

    // Apply translation
    transform[2][0] = position.x;
    transform[2][1] = position.y;

    return transform;
}

/**
 * Transform a local point to the world space in 2D
 */
inline glm::vec2 ToWorld(
    const glm::vec2 &point,
    const glm::vec2 &position,
    const glm::vec2 &heading,
    const glm::vec2 &scale)
{
    glm::mat3 transform = TransformMatrix(position, heading, scale);

    glm::vec3 local(point, 1.0f);
    glm::vec3 world = transform * local;
    return glm::vec2(world.x, world.y);
}

}  // steering
