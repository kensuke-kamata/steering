#include "Game.h"

int main (int argc, char* argv[]) {
  steering::Game game;

  if (game.Init()) {
    game.Mainloop();
  }
  game.Shutdown();

  return 0;
}
