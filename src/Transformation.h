#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Component.h"

namespace steering {
/**
 *
 */
inline glm::vec2 TransformToWorld(
        const glm::vec2 &target,
        const component::Transform *t) {
    glm::mat3 trasform = glm::mat3(1.0f);
    // transform = glm::rotate(transform, t->rotation)
    // transform = glm::translate(transform, glm::vec3());

    glm::vec2 point(0.0f, 0.0f);
    return point;
}

}  // steering
