#include "SceneFactory.hpp"

#include "MyGame.hpp"
#include "Scene/SampleScene.hpp"
#include "Utils.hpp"
#include "Scene/Game/GameScene.hpp"
#include "Scene/Title/TitleScene.hpp"

SceneFactory::SceneFactory(MyGame* _myGame) :myGame_(_myGame){
}

std::unique_ptr<IScene> SceneFactory::Create(const std::string& sceneName) {
    std::unique_ptr<IScene> scene = nullptr;
    if (Utils::EqualsIgnoreCase(sceneName, "sample")){
        scene = std::make_unique<SampleScene>();
    }else if (Utils::EqualsIgnoreCase(sceneName, "title")) {
        scene = std::make_unique<TitleScene>();
    }else if (Utils::EqualsIgnoreCase(sceneName, "game")) {
        scene = std::make_unique<GameScene>();
    }else {
        Utils::Alert(std::format("SceneFactory::Create: Scene '{}' not found", sceneName));
        return nullptr;
    }

    return scene;
}
