#ifndef STAGE_INFO_UI_HPP_
#define STAGE_INFO_UI_HPP_
#include <cstdint>
#include <functional>

#include "GEUI.hpp"

class UiTheme;

/** @brief ラウンドの目標（Objective）表示
 * 初回は画面中央に大きく出してから、同じ背景を左上へ縮めながら移動し、以降は左上に残り続ける。
 * 文字は拡縮できないため、移動の前後で大きい文字と小さい文字を入れ替える。
 */
class StageInfoUI {
    enum class Phase { Showing, Hold, HideLargeText, Moving, ShowSmallText, Done };

    /** @brief 見出しとゴールの文字の組 */
    struct TextGroup {
        GEUI::Widget label;
        GEUI::Widget goal;

        void SetAlpha(float _alpha, const UiTheme& _theme);
    };

    const UiTheme* theme_ = nullptr;

    GEUI::Canvas ui_;
    GEUI::Widget panel_;
    TextGroup large_;
    TextGroup small_;
    Vector2 centerOffset_{};

    Phase phase_   = Phase::Done;
    float elapsed_ = 0.f;

    std::function<void()> onMoveStart_;

public:
    ~StageInfoUI();

    void Initialize(const UiTheme& _theme);
    void Update();

    /** @brief 中央に大きく表示し、左上へ移動する演出を始める */
    void Setup(uint16_t _requireKill);

    /** @brief 演出なしで、最初から左上に表示する */
    void ShowAtCorner(uint16_t _requireKill);

    void SetActive(bool _active);

    /** @brief 背景が左上へ動き始めた瞬間に呼ばれる処理を設定する */
    void SetOnMoveStart(std::function<void()> _callback);

    /** @brief 背景が左上に着いたか（小さい文字のフェードイン中も含む） */
    bool IsFinish() const { return phase_ == Phase::ShowSmallText || phase_ == Phase::Done; }

private:
    void SetGoalText(uint16_t _requireKill);
    void ApplyMove(float _progress);
    void NextPhase(Phase _phase);

}; // class StageInfoUI

#endif // STAGE_INFO_UI_HPP_
