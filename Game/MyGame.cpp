#include "MyGame.hpp"

#include "Factory/SceneFactory.hpp"
#include "Factory/PostEffectFactory.hpp"

MyGame::MyGame(): IGame(std::make_unique<SceneFactory>(this), "game") {
    GetCurrentConfig()
        .SetTitle("GameTemplate")
        .SetFPS(60);

    // PostEffectFactoryを登録
    SetPostEffectFactory(std::make_unique<PostEffectFactory>());
}

MyGame::~MyGame() = default;

