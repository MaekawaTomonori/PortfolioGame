#include "SceneRegister.hpp"

#include "Scene/SampleScene.hpp"
#include "Scene/Game/GameScene.hpp"
#include "Scene/Result/GameClear.hpp"
#include "Scene/Result/GameOver.hpp"
#include "Scene/Title/TitleScene.hpp"

void SceneRegister::Register(const std::function<void(std::string, std::function<std::unique_ptr<IScene>()>)>& _func){
    _func("sample", [] {return std::make_unique<SampleScene>();});
    _func("title", []{return std::make_unique<TitleScene>();});
    _func("game", []{return std::make_unique<GameScene>();});
    _func("gameover", []{return std::make_unique<GameOver>();});
    _func("gameclear", []{return std::make_unique<GameClear>();});
}
