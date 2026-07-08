#ifndef StageInfoUI_HPP_
#define StageInfoUI_HPP_

#include <memory>

#include "Ui/UserInterface.hpp"

class StageInfoUI {
    enum class Phase { Showing, Hold, Hiding, Done };

    const float HOLD_DURATION    = 2.0f;
    const float FADE_DURATION    = 0.5f;

    Phase phase_   = Phase::Done;
    float elapsed_ = 0.f;

    std::unique_ptr<Ui::Canvas> ui_;

    void ApplyFadeFunc(bool _isFadeIn);

public:
    void Initialize();
    void Setup(float _time, uint16_t _requireKill);
    void Update(float _deltaTime);

    bool IsFinish() const { return phase_ == Phase::Done; }
}; // class StageInfoUI

#endif // StageInfoUI_HPP_
