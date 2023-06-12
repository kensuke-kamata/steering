#include "Game.h"

#include <cstdint>

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <ECS.h>

#include "Component.h"
#include "System.h"

namespace steering {

bool Game::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return false;
    }

    window_ = SDL_CreateWindow(
        "steering", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        static_cast<int>(SCREEN_W), static_cast<int>(SCREEN_H),
        SDL_WINDOW_OPENGL
    );
    if (window_ == nullptr) {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }

    renderer_ = SDL_CreateRenderer(
        window_, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (renderer_ == nullptr) {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        return false;
    }

    auto agent = scene_.NewEntity();
    // scene_.AddComponent<component::Seek>(agent);
    // scene_.AddComponent<component::Flee>(agent, 100.0f);
    scene_.AddComponent<component::Arrive>(agent, 2.0f);
    scene_.AddComponent<component::Triangle>(
        agent,
        15.0f // radius
    );
    scene_.AddComponent<component::Transform>(
        agent,
        glm::vec2(125.0f, 125.0f), // position
        glm::vec2(0.0f, -1.0f),    // rotation (head)
        glm::vec2(0.75f, 1.0f)     // scale
    );
    scene_.AddComponent<component::Move>(
        agent,
        glm::vec2(0.0f, 0.0f), // velocity
          1.0f, // mass
        150.0f, // max speed
         85.0f  // max force
    );
    scene_.AddComponent<component::Color>(agent, 255, 0, 0, 255);

    auto crosshair = scene_.NewEntity();
    scene_.AddComponent<component::Crosshair>(
        crosshair,
        5.0f
    );
    scene_.AddComponent<component::Transform>(
        crosshair,
        glm::vec2(250.0f, 250.0f),
        glm::vec2(0.0f, -1.0f),
        glm::vec2(1.0f, 1.0f)
    );
    scene_.AddComponent<component::Color>(crosshair, 0, 0, 0, 255);

    return true;
}

void Game::Shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

void Game::Mainloop() {
    ticks_   = SDL_GetTicks();
    running_ = true;
    while (running_) {
        ProcessInput();
        Update();
        Draw();
    }
}

void Game::ProcessInput() {
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running_ = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&mouse_.x, &mouse_.y);
                break;
        }
    }

    const auto *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_ESCAPE]) {
        running_ = false;
    }
}

void Game::Update() {
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticks_ + 16));

    float dt = (SDL_GetTicks() - ticks_) / 1000.0f;
    if (profile_) {
        SDL_Log("FPS: %f", 1.0f / dt);
    }
    dt = glm::clamp<float>(dt, 0.0f, 0.05f);


    update::Crosshair(glm::vec2(mouse_.x, mouse_.y), scene_);
    update::Wraparound(SCREEN_W, SCREEN_H, scene_);

    behavior::Seek(glm::vec2(mouse_.x, mouse_.y), scene_, dt);
    behavior::Flee(glm::vec2(mouse_.x, mouse_.y), scene_, dt);
    behavior::Arrive(glm::vec2(mouse_.x, mouse_.y), scene_, dt);

    ticks_ = SDL_GetTicks();
}

void Game::Draw() {
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer_);

    draw::Crosshair(renderer_, scene_);
    draw::Triangle(renderer_, scene_);

    SDL_RenderPresent(renderer_);
}

}  // steering steering
