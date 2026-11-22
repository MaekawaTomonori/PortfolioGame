#ifndef PLAY_HUD_HPP_
#define PLAY_HUD_HPP_
#include <cstdint>
#include <memory>

#include "Ui/GameTimer/GameTimer.hpp"
#include "Ui/KeyGuide/KeyGuide.hpp"
#include "Ui/KillCount/KillCounter.hpp"
#include "Ui/StageInfo/StageInfoUI.hpp"

class Enemies;
class SkillManager;

/** @brief プレイ中に出す HUD（残り時間・キー案内・撃破数・目標）をまとめて扱う */
class PlayHud {
    std::unique_ptr<GameTimer> gameTimer_;
    std::unique_ptr<KeyGuide> keyGuide_;
    std::unique_ptr<KillCounter> killCounter_;
    std::unique_ptr<StageInfoUI> stageInfo_;

public:
    void Initialize(const KeyBinds& _binds, const UiTheme& _theme);
    void Update();
    void Debug();

    /** @brief 1ラウンド目。目標を中央に出して左上へ動かし、動き始めたら撃破数を出す */
    void PlayIntro(uint16_t _requirementKill, Enemies* _enemies, float _duration);

    /** @brief 2ラウンド目以降。目標と撃破数を最初から左上に出す */
    void StartRound(uint16_t _requirementKill, Enemies* _enemies, float _duration);

    /** @brief キー案内を出し、残り時間を動かし始める */
    void Show();
    void Hide();

    void SetCooldowns(const SkillManager& _skillManager);

    /** @brief 目標が左上に着いたか */
    bool IsIntroFinished() const;
    bool IsTimeUp() const;
}; // class PlayHud

#endif // PLAY_HUD_HPP_
