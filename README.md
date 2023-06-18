# Steering

## System Requirements

### Libraries
- SDL2
- glm

## Behaviors

All agents that exhibit steering behavior must have the `Transform` and `Move` components added. Additionally, one of the following steering components should be included.

```c++
// Example

#include <ECS.h>
ecs::Scene scene;

auto agent = scene.NewEntity();
scene.AddComponent<component::Seek>(agent);
scene.AddComponent<component::Triangle>(
    agent,
    15.0f  // radius
);
scene.AddComponent<component::Transform>(
    agent,
    glm::vec2(125.0f, 125.0f), // position
    glm::vec2(0.0f, -1.0f),    // rotation (head)
    glm::vec2(0.75f, 1.0f)     // scale
);
scene.AddComponent<component::Move>(
    agent,
    glm::vec2(0.0f, 0.0f), // velocity
      1.0f, // mass
    150.0f, // max speed
     85.0f  // max force
);
```

### `Seek`

```c++
// Defined Component
struct Seek {};
```

```c++
// System interface
inline void Seek(glm::vec2 target, ecs::Scene &scene, float dt);
```

### `Flee`

```c++
// Defined Component
struct Flee {
    Flee() = default;
    Flee(float radius) : radius(radius) {}

    float radius{ 100.0f };
};
```

```c++
// System interface
inline void Flee(glm::vec2 target, ecs::Scene &scene, float dt);
```

### `Arrive`

```c++
// Defined Component
struct Arrive {
    Arrive() = default;
    Arrive(float dec) : deceleration(dec) {}

    float deceleration{ 2.0f };
};
```

```c++
// System interface
inline void Arrive(glm::vec2 target, ecs::Scene &scene, float dt);
```

### `Pursuit`

```c++
// Defined Component
struct Pursuit {
    Pursuit() = default;
    Pursuit(ecs::Entity::Id evaderId) : evaderId(evaderId) {}

    ecs::Entity::Id evaderId = -1;
};
```

```c++
// System interface
inline void Arrive(ecs::Scene &scene, float dt);
```

### `Evade`

```c++
// Defined Component
struct Evade {
    Evade() = default;
    Evade(ecs::Entity::Id pursuerId, float radius)
        : pursuerId(pursuerId),
          radius(radius) {}

    ecs::Entity::Id pursuerId{ static_cast<ecs::Entity::Id>(-1) };
    float radius{ 100.0f };
};
```

```c++
// System interface
inline void Evade(ecs::Scene &scene, float dt);
```

### `Wander`

```c++
// Defined Component
struct Wander {
    Wander(ecs::Entity::Id target, ecs::Entity::Id forward,
           float radius, float distance, float jitter)
        : target(target),
          forward(forward),
          distance(distance),
          jitter(jitter) {}

    ecs::Entity::Id target;
    ecs::Entity::Id forward;

    glm::vec2 point{ glm::vec2(0.0f) };
    float radius{ 25.0f };
    float distance{ 100.0f };
    float jitter{ 5.0f };
};
```

```c++
// System interface
inline void Wander(ecs::Scene &scene, float dt);
```
