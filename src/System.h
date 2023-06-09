#pragma once

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <ECS.h>

#include "Component.h"

namespace steering {

namespace draw {
/**
 * Draw triangles in an SDL rendering context.
 */
inline void Triangle(SDL_Renderer *renderer, ecs::Scene &scene) {
    for (auto id : ecs::SceneView<component::Triangle,
                                  component::Transform,
                                  component::Color>(scene)) {
        auto triangle = scene.GetComponent<component::Triangle>(id);
        auto radius = triangle->radius;

        auto transform = scene.GetComponent<component::Transform>(id);
        auto   pos = transform->position;
        auto  head = transform->rotation;
        auto scale = transform->scale;
        auto  side = glm::vec2(-head.y, head.x);

        auto p1 = pos + head * radius * scale.y;
        auto p2 = pos - head * radius + side * radius * scale.x;
        auto p3 = pos - head * radius - side * radius * scale.x;

        auto color = scene.GetComponent<component::Color>(id);
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
        SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
        SDL_RenderDrawLine(renderer, p2.x, p2.y, p3.x, p3.y);
        SDL_RenderDrawLine(renderer, p3.x, p3.y, p1.x, p1.y);
    }
}

/**
 * Draw crosshairs in an SDL rendering context.
 */
inline void Crosshair(SDL_Renderer *renderer, ecs::Scene &scene) {
    for (auto id : ecs::SceneView<component::Crosshair,
                                  component::Transform,
                                  component::Color>(scene)) {
        auto crosshair = scene.GetComponent<component::Crosshair>(id);
        auto radius = crosshair->radius;

        auto transform = scene.GetComponent<component::Transform>(id);
        auto   pos = transform->position;
        auto scale = transform->scale;

        auto p1 = glm::vec2(pos.x + radius, pos.y) * scale;
        auto p2 = glm::vec2(pos.x, pos.y + radius) * scale;
        auto p3 = glm::vec2(pos.x - radius, pos.y) * scale;
        auto p4 = glm::vec2(pos.x, pos.y - radius) * scale;

        auto color = scene.GetComponent<component::Color>(id);
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
        SDL_RenderDrawLine(renderer, p1.x, p1.y, p3.x, p3.y);
        SDL_RenderDrawLine(renderer, p2.x, p2.y, p4.x, p4.y);
    }
}
}  // draw

namespace update {
/**
 * Update the position of a crosshair.
 * The new target position is provided as an argument to this function.
 */
inline void Crosshair(glm::vec2 target, ecs::Scene &scene) {
    for (auto id : ecs::SceneView<component::Crosshair,
                                  component::Transform>(scene)) {
        auto transform = scene.GetComponent<component::Transform>(id);
        transform->position.x = target.x;
        transform->position.y = target.y;
    }
}

/**
 * Update the position for wraparound effect within
 * the boundaries of the screen width/height.
 */
inline void Wraparound(int screenW, int screenH, ecs::Scene &scene) {
    for (auto id : ecs::SceneView<component::Transform>(scene)) {
        auto transform = scene.GetComponent<component::Transform>(id);

        auto maxX = static_cast<float>(screenW);
        auto maxY = static_cast<float>(screenH);

        // Wraparound X
        if (maxX < transform->position.x) {
            transform->position.x = 0.0f;
        } else if (transform->position.x < 0.0f) {
            transform->position.x = maxX;
        }

        // Wraparound Y
        if (maxY < transform->position.y) {
            transform->position.y = 0.0f;
        } else if (transform->position.y < 0.0f) {
            transform->position.y = maxY;
        }
    }
}
}  // update

namespace behavior {
/**
 * Seek behavior for entities.
 */
inline void Seek(glm::vec2 target, ecs::Scene &scene, float dt) {
    for (auto id : ecs::SceneView<component::Seek,
                                  component::Transform,
                                  component::Move>(scene)) {
        auto m = scene.GetComponent<component::Move>(id);
        auto t = scene.GetComponent<component::Transform>(id);

        // Calculate the direction and distance to the target.
        auto direct = target - t->position;
        auto dist = glm::length(direct);
        if (dist < glm::epsilon<float>()) {
            continue;
        }
        direct /= dist;

        // Compute the desired velocity and steering force.
        auto velocity = direct * m->maxSpeed;
        auto steering = velocity - m->velocity;

        // Limit the steering force to the maximum allowed force.
        // This help to create smooth movement, preventing the entity from
        // instantly turning around to face the target.
        auto lenS = glm::length(steering);
        if (m->maxForce < lenS) {
            steering /= lenS;
            steering *= m->maxForce;
        }

        // Calculate acceleration based on the steering force.
        auto acc = steering / m->mass;
        m->velocity += acc * dt;

        // Limit the entity's speed to its maxmum speed.
        auto lenV1 = glm::length(m->velocity);
        if (m->maxSpeed < lenV1) {
            m->velocity /= lenV1;
            m->velocity *= m->maxSpeed;
        }

        // Update the entity's position.
        t->position += m->velocity * dt;

        // If the entity is stationary, skip the rotation step.
        auto lenV2 = glm::length(m->velocity);
        if (lenV2 < glm::epsilon<float>()) {
            return;
        }
        t->rotation = glm::normalize(m->velocity);
    }
}

/**
 * Flee behavior for entities.
 */
inline void Flee(glm::vec2 target, ecs::Scene &scene, float dt) {
    for (auto id : ecs::SceneView<component::Flee,
                                  component::Transform,
                                  component::Move>(scene)) {
        auto f = scene.GetComponent<component::Flee>(id);
        auto m = scene.GetComponent<component::Move>(id);
        auto t = scene.GetComponent<component::Transform>(id);

        auto direct = t->position - target;  // Flee direction from the target
        auto dist = glm::length(direct);
        if (dist < glm::epsilon<float>()) {
            continue;
        }
        direct /= dist;

        auto velocity = direct * m->maxSpeed;
        auto steering = velocity - m->velocity;

        auto lenS = glm::length(steering);
        if (m->maxForce < lenS) {
            steering /= lenS;
            steering *= m->maxForce;
        }

        // Zero steering force if the target isn't in the escape radius
        if (f->radius < dist) {
            steering = glm::vec2(0.0f);
        }
        auto acc = steering / m->mass;
        m->velocity += acc * dt;

        auto lenV1 = glm::length(m->velocity);
        if (m->maxSpeed < lenV1) {
            m->velocity /= lenV1;
            m->velocity *= m->maxSpeed;
        }

        t->position += m->velocity * dt;

        auto lenV2 = glm::length(m->velocity);
        if (lenV2 < glm::epsilon<float>()) {
            return;
        }
        t->rotation = glm::normalize(m->velocity);
    }
}
}  // behavior

}  // steering
