#include "StageInfoUI.hpp"

#include <algorithm>
#include "src/Time/Time.hpp"

void StageInfoUI::Initialize() {
    ui_ = std::make_unique<Ui::Canvas>();
    ui_->Setup("stageinfo");
    ui_->SetActive(false);
}

void StageInfoUI::Setup(const float _time, const uint16_t _requireKill) {
    if (auto* elem = ui_->FindElementByName("time")) {
        elem->SetText("Time Limit: " + std::to_string(static_cast<int>(_time)) + " s");
    }
    if (auto* elem = ui_->FindElementByName("kill")) {
        elem->SetText("Goal: Defeat " + std::to_string(_requireKill) + " enemies");
    }

    ApplyFadeFunc(true);

    ui_->SetActive(true);
    phase_   = Phase::Showing;
    elapsed_ = 0.f;
}

void StageInfoUI::Update() {
    float deltaTime = Time::GetDeltaTime();

    switch (phase_) {
    case Phase::Showing:
        elapsed_ += deltaTime;
        if (elapsed_ >= FADE_DURATION) {
            phase_   = Phase::Hold;
            elapsed_ = 0.f;
        }
        break;

    case Phase::Hold:
        elapsed_ += deltaTime;
        if (elapsed_ >= HOLD_DURATION) {
            ApplyFadeFunc(false);
            phase_   = Phase::Hiding;
            elapsed_ = 0.f;
        }
        break;

    case Phase::Hiding:
        elapsed_ += deltaTime;
        if (elapsed_ >= FADE_DURATION) {
            // SetActive(false) 内部の PlayHide() が即完了するようにヒド関数を解除
            for (size_t i = 0; i < ui_->GetElementCount(); ++i) {
                if (auto* elem = ui_->GetElement(i)) elem->SetHideFunc({});
            }
            ui_->SetActive(false);
            phase_ = Phase::Done;
        }
        break;

    case Phase::Done:
        break;
    }
}

void StageInfoUI::ApplyFadeFunc(const bool _isFadeIn) {
    const float duration = FADE_DURATION;

    for (size_t i = 0; i < ui_->GetElementCount(); ++i) {
        auto* elem = ui_->GetElement(i);
        if (!elem) continue;

        if (_isFadeIn) {
            elem->SetShowFunc([duration](float elapsed, Vector2&, Vector4& color) -> bool {
                const float t = std::min(elapsed / duration, 1.f);
                color.w *= t;
                return t >= 1.f;
            });
            elem->PlayShow();
        } else {
            elem->SetHideFunc([duration](float elapsed, Vector2&, Vector4& color) -> bool {
                const float t = std::min(elapsed / duration, 1.f);
                color.w *= (1.f - t);
                return t >= 1.f;
            });
            elem->PlayHide();
        }
    }
}
