#ifndef GameScene_HPP_
#define GameScene_HPP_
#include "IScene.hpp"

#include "FollowCamera.hpp"
#include "Camera/Director/CameraDirector.hpp"
#include "Stage/Stage.hpp"

class GameScene : public IScene{
    CameraDirector* camerawork_ = nullptr;

    std::unique_ptr<FollowCamera> followCamera_;
    std::unique_ptr<Stage> stage_;

    bool intro_ = true;
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

}; // class GameScene

#endif // GameScene_HPP_
