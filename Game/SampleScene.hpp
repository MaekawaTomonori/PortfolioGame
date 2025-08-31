#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"
#include "Sprite.hpp"
#include "src/Sky/Skybox.hpp"
#include "Player/Player.hpp"
#include "CameraController.hpp"
#include "Enemy/Enemy.hpp"

class SampleScene final : public IScene{
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Model> model_;
    std::unique_ptr<Skybox> sky_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<CameraController> cameraController_;

    std::unique_ptr<Enemy> enemy_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:

}; // class SampleScene

#endif // SampleScene_HPP_
