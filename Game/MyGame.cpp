#include "MyGame.hpp"

#include "SceneFactory.hpp"

MyGame::MyGame(): IGame(std::make_unique<SceneFactory>(this), "game") {
    GetCurrentConfig()
        .SetTitle("GameTemplate")
        .SetFPS(60);
}

MyGame::~MyGame() = default;

