#pragma once

#include <random>

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <ECS.h>

#include "Component.h"
#include "Transformation.h"

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

inline void Circle(SDL_Renderer *renderer, ecs::Scene &scene) {
    for (auto id : ecs::SceneView<component::Circle,
                                  component::Transform,
                                  component::Color>(scene)) {
        auto circle = scene.GetComponent<component::Circle>(id);
        auto radius = circle->radius;

        auto transform = scene.GetComponent<component::Transform>(id);
        auto   pos = transform->position;
        auto scale = transform->scale;

        auto color = scene.GetComponent<component::Color>(id);
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

        // Based on x
        for (auto x = -radius; x <= radius; x += 1.0f) {
            auto y = glm::sqrt(radius * radius - x * x);
            auto px = pos.x + x;
            {
                // Upper half
                auto py = pos.y - y;
                SDL_RenderDrawPoint(renderer, px, py);
            }
            {
                // Lower half
                auto py = pos.y + y;
                SDL_RenderDrawPoint(renderer, px, py);
            }
        }

        // Based on y
        for (auto y = -radius; y <= radius; y += 1.0f) {
            auto x = glm::sqrt(radius * radius - y * y);
            auto py = pos.y + y;
            {
                // Left half
                auto px = pos.x - x;
                SDL_RenderDrawPoint(renderer, px, py);
            }
            {
                // Right half
                auto px = pos.x + x;
                SDL_RenderDrawPoint(renderer, px, py);
            }
        }
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

/**
 * Arrive behavior for entities.
 */
inline void Arrive(glm::vec2 target, ecs::Scene &scene, float dt) {
    for (auto id : ecs::SceneView<component::Arrive,
                                  component::Transform,
                                  component::Move>(scene)) {
        auto a = scene.GetComponent<component::Arrive>(id);
        auto m = scene.GetComponent<component::Move>(id);
        auto t = scene.GetComponent<component::Transform>(id);

        auto direct = target - t->position;
        auto dist = glm::length(direct);

        auto steering = glm::zero<glm::vec2>();
        if (glm::epsilon<float>() < dist) {
            auto speed = static_cast<float>(dist / (a->deceleration));
            speed = glm::min<float>(speed, m->maxSpeed);

            auto velocity = direct * speed / dist;
            steering = velocity - m->velocity;
        }

        auto lenS = glm::length(steering);
        if (m->maxForce < lenS) {
            steering /= lenS;
            steering *= m->maxForce;
        }

        auto acc = steering / m->mass;
        m->velocity += acc * dt;

        auto lenV1 = glm::length(m->velocity);
        if (m->maxSpeed < lenV1) {
            m->velocity /= lenV1;
            m->velocity *= m->maxSpeed;
        }

        auto lenV2 = glm::length(m->velocity);
        if (lenV2 < 10.0f) {
            // No need to adjust position or rotation if the agent is within
            // a certain proximity to the target.
            return;
        }
        t->position += m->velocity * dt;
        t->rotation = glm::normalize(m->velocity);
    }
}

/**
 * Calculate the time an agent requires to reorient itself towards a target.
 */
inline float TurnaroundTime(component::Transform *pTransform,
                            component::Transform *eTransform) {
    auto to = glm::normalize(eTransform->position - pTransform->position);
    auto dot = glm::dot(pTransform->rotation, to);

    const float coefficient = 0.5f;
    return (dot - 1.0f) * -coefficient;
}

/**
 * Pursuit behavior for entities.
 */
inline void Pursuit(ecs::Scene &scene, float dt) {
    for (auto id : ecs::SceneView<component::Pursuit,
                                  component::Transform,
                                  component::Move>(scene)) {
        auto p = scene.GetComponent<component::Pursuit>(id);
        auto t = scene.GetComponent<component::Transform>(id);
        auto m = scene.GetComponent<component::Move>(id);

        if (!ecs::Entity::IsValid(p->evaderId)) {
            continue;
        }
        if (!scene.HasComponent<component::Transform>(p->evaderId) ||
            !scene.HasComponent<component::Move>(p->evaderId)) {
            continue;
        }
        auto et = scene.GetComponent<component::Transform>(p->evaderId);
        auto em = scene.GetComponent<component::Move>(p->evaderId);

        auto to = et->position - t->position;
        auto dot = glm::dot(t->rotation, et->rotation);

        // Init the target with the evader's position
        auto target = et->position;
        // If the evader is ahead of the pursuer and
        // both are heading the same direction within 18 degrees (acos(dot) = 0.95)
        // just seek to the evader's position

        // If the evader isn't ahead so guessing where it is.
        // How long the pursuer can predict the evader's future position
        // is proportional to the distance between them and
        // inversaly proportional to the sum of their speeds.
        auto angle = glm::acos(dot) * 180 / glm::pi<float>();
        auto dot2 = glm::dot(t->rotation, to);
        if ((dot2 < 0) || dot < 0.95) {
            SDL_Log("Pursuit: By Predict: dot2 %f", dot2);
            SDL_Log("Pursuit: By Predict: angle %f", angle);
            auto lookaheadtime = glm::length(to) / (m->maxSpeed + em->maxSpeed);
            lookaheadtime += TurnaroundTime(t, et);
            target = et->position + em->velocity * lookaheadtime;
        } else {
            SDL_Log("Pursuit: By Seek: dot2 %f", dot2);
            SDL_Log("Pursuit: By Seek: angle: %f", angle);
        }

        // TODO: Computing below is the same as the seek, so should be replaced
        //       to a new common function.
        auto direct = target - t->position;
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

/**
 * Evade behavior for entities.
 */
inline void Evade(ecs::Scene &scene, float dt) {
    for (auto id : ecs::SceneView<component::Evade,
                                  component::Transform,
                                  component::Move>(scene)) {
        auto e = scene.GetComponent<component::Evade>(id);
        auto t = scene.GetComponent<component::Transform>(id);
        auto m = scene.GetComponent<component::Move>(id);

        if (!ecs::Entity::IsValid(e->pursuerId)) {
            continue;
        }
        if (!scene.HasComponent<component::Transform>(e->pursuerId) ||
            !scene.HasComponent<component::Move>(e->pursuerId)) {
            continue;
        }
        auto pt = scene.GetComponent<component::Transform>(e->pursuerId);
        auto pm = scene.GetComponent<component::Move>(e->pursuerId);

        auto to = pt->position - t->position;
        auto lookaheadtime = glm::length(to) / (m->maxSpeed + pm->maxSpeed);
        auto target = pt->position + pm->velocity * lookaheadtime;

        // TODO: The same process of Flee
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
        if (e->radius < dist) {
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

/**
 *
 */
inline float RandomClamped() {
    static std::default_random_engine engine(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(engine);
}

/**
 * Wander behavior for entities.
 */
inline void Wander(ecs::Scene &scene, float dt) {
    for (auto id : ecs::SceneView<component::Wander,
                                  component::Transform,
                                  component::Move>(scene)) {
        auto w = scene.GetComponent<component::Wander>(id);
        auto t = scene.GetComponent<component::Transform>(id);
        auto m = scene.GetComponent<component::Move>(id);

        auto  targetCircle = w->target;
        auto forwardCircle = w->forward;

        auto tt = scene.GetComponent<component::Transform>(targetCircle);
        auto ft = scene.GetComponent<component::Transform>(forwardCircle);
        auto fc = scene.GetComponent<component::Circle>(forwardCircle);

        auto randomX = RandomClamped() * w->jitter * dt;
        auto randomY = RandomClamped() * w->jitter * dt;
        w->point += glm::vec2(randomX, randomY);
        w->point  = glm::normalize(w->point);
        w->point *= w->radius;

        auto targetLocal = w->point + glm::vec2(w->distance, 0);
        auto targetWorld = ToWorld(targetLocal, t->position, t->rotation, glm::vec2(1.0f, 1.0f));

        auto direct = targetWorld - t->position;
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

        fc->radius   = w->radius;
        ft->position = t->position + t->rotation * w->distance;
        tt->position = targetWorld;
    }
}

}  // behavior
}  // steering
