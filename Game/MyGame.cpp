#include "MyGame.hpp"

#include "SceneRegister.hpp"
#include "Factory/PostEffectFactory.hpp"

void MyGame::Initialize(GameEngine::Config& _config) {
    _config = { 
        .title = "GameTemplate",
        .defaultScene = "sample",
        .fps = 60,
    };

    SceneRegister::Register([this](const std::string& _name, std::function<std::unique_ptr<IScene>()> _func) {
        RegisterScene(_name, _func);
    });

    // PostEffectFactoryを登録
    SetPostEffectFactory(std::make_unique<PostEffectFactory>());    
}
