#include "MyGame.hpp"

#include "PostProcess/BoxBlur/BoxBlur.hpp"
#include "PostProcess/Grayscale/Grayscale.hpp"
#include "PostProcess/Vignette/Vignette.hpp"
#include "Scene/Particle/ParticleScene.hpp"
#include "Scene/Title/TitleScene.hpp"
#include "Scene/Game/GameScene.hpp"
#include "Scene/Result/GameClear.hpp"
#include "Scene/Result/GameOver.hpp"

void MyGame::Initialize(GameEngine::Config& _config) {
    // ゲーム固有の設定（Assets/Config/App.cnf に含まれないもの）
    _config.defaultScene = "title";

    Register();
}

void MyGame::Register(){
    RegisterScene<TitleScene>("title");
    RegisterScene<GameScene>("game");
    RegisterScene<GameOver>("gameover");
    RegisterScene<GameClear>("gameclear");
    RegisterScene<ParticleScene>("particle");

    // presets.json等から型名で参照されるため、GetTypeName()と同じ名前で登録する
    RegisterPostEffect<Vignette>("Vignette");
    RegisterPostEffect<Grayscale>("Grayscale");
    RegisterPostEffect<BoxBlur>("BoxBlur");
}