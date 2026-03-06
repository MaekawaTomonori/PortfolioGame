#include "MyGame.hpp"

#include "Factory/PostEffectFactory.hpp"
#include "Scene/SampleScene.hpp"
#include "Scene/Game/GameScene.hpp"
#include "Scene/Result/GameOver.hpp"
#include "Scene/Title/TitleScene.hpp"

void MyGame::Initialize(GameEngine::Config& _config) {
    _config = { 
        .title = "DarkRun",
        .defaultScene = "title",
        .fps = 60,
        .showCursor = false,
    };

    Register();

    // PostEffectFactoryを登録
    SetPostEffectFactory(std::make_unique<PostEffectFactory>());    
}

void MyGame::Register(){
    RegisterScene<SampleScene>("sample");
    RegisterScene<TitleScene>("title");
    RegisterScene<GameScene>("game");
    RegisterScene<GameOver>("gameover");
    RegisterScene<GameClear>("gameclear");
}