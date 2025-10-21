#ifndef GameScene_HPP_
#define GameScene_HPP_
#include "IScene.hpp"

#include "FollowCamera.hpp"
#include "Performance/Intro/Intro.hpp"
#include "Stage/Stage.hpp"

class GameScene : public IScene{
    std::unique_ptr<FollowCamera> followCamera_;
    std::unique_ptr<Stage> stage_;

    std::unique_ptr<Intro> intro_;
    bool introD_ = false;

    std::unique_ptr<Sprite> sprite_ ;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

}; // class GameScene

#endif // GameScene_HPP_
