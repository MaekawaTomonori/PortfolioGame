#ifndef GameScene_HPP_
#define GameScene_HPP_
#include "IScene.hpp"

#include "FollowCamera.hpp"
#include "Performance/Intro/Intro.hpp"
#include "Player/KeyBinds.hpp"
#include "Stage/Stage.hpp"
#include "Status/GameStatus.hpp"
#include "Ui/Pause/Pause.hpp"
#include "Ui/PlayHud.hpp"
#include "Ui/SkillTree/SkillTree.hpp"
#include "Ui/UiTheme.hpp"

class GameScene : public IScene {
    enum State {
        INTRO,
        PLAY,
        UPGRADE,
        OUTRO
    };

    State state_ = INTRO;

    KeyBinds binds_;
    UiTheme theme_;

    std::unique_ptr<FollowCamera> followCamera_;
    std::unique_ptr<Stage> stage_;

    std::unique_ptr<Intro> intro_;

    std::unique_ptr<SkillTree> skillTree_;

    // Status --
    GameStatus status_ {};

    std::unique_ptr<PlayHud> hud_;
    std::unique_ptr<Pause> pause_;

    bool cursorShown_ = false;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    void Debug() override;

private:
    void OnEnable() override;

    /** @brief Pause か SkillTree が開いている間だけ、マウスカーソルを出す */
    void UpdateCursorVisibility();

    void UpdatePlay();

    /** @brief ステージを初期化し、次のラウンドをPLAY状態で開始する */
    void StartNextRound();

}; // class GameScene

#endif // GameScene_HPP_
