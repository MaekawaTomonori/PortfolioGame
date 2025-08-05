#include "SceneFactory.hpp"

#include "SampleScene.hpp"
#include "Utils.hpp"

std::unique_ptr<IScene> SceneFactory::Create(const std::string& sceneName) {
    std::unique_ptr<IScene> scene = nullptr;
    if (Utils::EqualsIgnoreCase(sceneName, "sample")){
        scene = std::make_unique<SampleScene>();
    }

    return scene;
}
