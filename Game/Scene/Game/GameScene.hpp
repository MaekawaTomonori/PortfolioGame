#ifndef GameScene_HPP_
#define GameScene_HPP_
#include "IScene.hpp"

#include "FollowCamera.hpp"
#include "GameTimer/GameTimer.hpp"
#include "Outro/Outro.hpp"
#include "Performance/Intro/Intro.hpp"
#include "Stage/Stage.hpp"
#include "Status/GameStatus.hpp"
#include "Ui/KeyGuide/KeyGuide.hpp"
#include "Ui/KillCount/KillCounter.hpp"
#include "Ui/Pause/Pause.hpp"
#include "Ui/SkillTree/SkillTree.hpp"

class GameScene : public IScene {
    enum State {
        INTRO,
        PLAY,
        UPGRADE,
        OUTRO
    };

    State state_ = INTRO;

    std::unique_ptr<FollowCamera> followCamera_;
    std::unique_ptr<Stage> stage_;

    std::unique_ptr<Intro> intro_;

    std::unique_ptr<SkillTree> skillTree_;

    Collision::Manager* cManager_ = nullptr;

    std::unique_ptr<Outro> outro_;
    bool clear_ = false;
    
    // Status --
    GameStatus status_ {};

    std::unique_ptr<GameTimer> gameTimer_;

    std::unique_ptr<Pause> pause_;

    std::unique_ptr<KeyGuide> keyGuide_;

    std::unique_ptr<KillCounter> killCounter_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;

private:

}; // class GameScene

#endif // GameScene_HPP_
