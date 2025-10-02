#ifndef SceneFactory_HPP_
#define SceneFactory_HPP_
#include "Factory/AbstractSceneFactory.hpp"

class MyGame;

class SceneFactory final : public AbstractSceneFactory{
    MyGame* myGame_ = nullptr;
public:
    SceneFactory(MyGame* _myGame);
    ~SceneFactory() override = default;
    std::unique_ptr<IScene> Create(const std::string& sceneName) override;
}; // class SceneFactory

#endif // SceneFactory_HPP_
