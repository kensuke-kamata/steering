#pragma once

#include <unordered_map>
#include <memory>
#include <vector>

#include <SDL.h>
#include <glm/glm.hpp>

#include <ECS.h>

namespace steering {

constexpr unsigned int SCREEN_W(500);
constexpr unsigned int SCREEN_H(500);

class Game {
public:
    Game() {};
    bool Init();
    void Shutdown();

    void Mainloop();
    void ProcessInput();
    void Update();
    void Draw();

private:
    SDL_Window     *window_{ nullptr };
    SDL_Renderer *renderer_{ nullptr };

    ecs::Scene scene_;

    uint32_t ticks_{ 0 };
    bool   running_{ false };
    bool  updating_{ false };
    bool   profile_{ true };

    struct MouseState {
        int x{ SCREEN_W / 2 };
        int y{ SCREEN_H / 2 };
    };
    MouseState mouse_{};
};

}  // namespace steering
