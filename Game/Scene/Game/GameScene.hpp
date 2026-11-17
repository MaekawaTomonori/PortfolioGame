#ifndef GameScene_HPP_
#define GameScene_HPP_
#include "IScene.hpp"

#include "FollowCamera.hpp"
#include "Ui/GameTimer/GameTimer.hpp"
#include "Performance/Intro/Intro.hpp"
#include "Player/KeyBinds.hpp"
#include "Stage/Stage.hpp"
#include "Status/GameStatus.hpp"
#include "Ui/KeyGuide/KeyGuide.hpp"
#include "Ui/KillCount/KillCounter.hpp"
#include "Ui/Pause/Pause.hpp"
#include "Ui/SkillTree/SkillTree.hpp"
#include "Ui/StageInfo/StageInfoUI.hpp"
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

    std::unique_ptr<GameTimer> gameTimer_;

    std::unique_ptr<Pause> pause_;

    std::unique_ptr<KeyGuide> keyGuide_;
    std::unique_ptr<KillCounter> killCounter_;
    std::unique_ptr<StageInfoUI> stageInfoUI_;

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

    /** @brief プレイ開始時のHUD（キー案内・タイマー）を表示し、登場演出を流す */
    void ShowPlayHud();

    /** @brief プレイ中のHUDを隠す */
    void HidePlayHud();

    void LoadStatus();
    void SaveStatus();

}; // class GameScene

#endif // GameScene_HPP_
