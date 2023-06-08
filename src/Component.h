#pragma once

#include <glm/glm.hpp>

namespace steering {
namespace component {

struct Transform {
    Transform() = default;
    Transform(glm::vec2 position, glm::vec2 rotation, glm::vec2 scale)
            : position(position),
              rotation(rotation),
              scale(scale) {}

    glm::vec2 position;
    glm::vec2 rotation;
    glm::vec2 scale;
};

struct Move {
    Move() = default;
    Move(glm::vec2 velocity, float mass, float maxSpeed, float maxForce)
            : velocity(velocity),
              mass(mass),
              maxSpeed(maxSpeed),
              maxForce(maxForce) {}

    glm::vec2 velocity;
    float mass;
    float maxSpeed; // max length of the velocity can be moved per time
    float maxForce; // max length of the steering force applied to the entity per time
};

struct Color {
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct Shape {
    Shape() = default;
    Shape(float radius) : radius(radius) {}

    float radius;
};

struct Triangle : Shape {
    Triangle() = default;
    Triangle(float radius) : Shape(radius) {}
};

struct Crosshair : Shape {
    Crosshair() = default;
    Crosshair(float radius) : Shape(radius) {}
};

struct Seek {};

}  // component
}  // steering
