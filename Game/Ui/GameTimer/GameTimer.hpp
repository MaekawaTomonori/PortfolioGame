#ifndef GameTimer_HPP_
#define GameTimer_HPP_
#include "GEUI.hpp"

class GameTimer {
    GEUI::Canvas ui_;
    GEUI::Widget root_;
    GEUI::Widget text_;
    GEUI::Widget fill_;

    float duration_ = 0.f;
    float time_ = 0.f;
    bool done_ = false;
    // デバッグ用。残り時間の進みだけを止める
    bool paused_ = false;

    float fadeElapsed_ = 0.f;

public:
    ~GameTimer();

    void Initialize();
    void Update();
    void Debug();

    bool IsDone() const { return done_; }

    void SetDuration(float _sec);
    void SetActive(bool _active);

    void PlayIntroAnim();

private:
    void ApplyTime();

}; // class GameTimer

#endif // GameTimer_HPP_
