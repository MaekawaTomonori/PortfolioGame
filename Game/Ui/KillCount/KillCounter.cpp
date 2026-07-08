#define NOMINMAX
#include "KillCounter.hpp"

#include <algorithm>

#include "Enemy/Enemies.hpp"
#include "Math/Easing.hpp"

void KillCounter::Initialize(Enemies* _enemies, const uint16_t _max) {
    enemies_ = _enemies;
    max_ = _max;

    ui_ = std::make_unique<Ui::Canvas>();
    ui_->Setup("killcounter");
}

void KillCounter::Update() {
    if (!ui_ || !enemies_) return;

    uint16_t current = std::min(max_, enemies_->GetDeathCount());

    auto setDigit = [this](const std::string& _name, int _digit) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        elem->SetTextureRegion({static_cast<float>(_digit % 10) * TEXTURE_SIZE.x, 0.f}, TEXTURE_SIZE);
    };

    setDigit("current_d0", current / 10);
    setDigit("current_d1", current % 10);
    setDigit("max_d0", max_ / 10);
    setDigit("max_d1", max_ % 10);
}

void KillCounter::Draw() const {
}

void KillCounter::Debug() {
}

void KillCounter::SetActive(bool _active) {
    if (ui_) ui_->SetActive(_active);
}

void KillCounter::PlayIntroAnim() {
    if (!ui_) return;

    const float    duration = 0.8f;
    const Vector2  center   = { 640.f, 360.f };

    for (size_t i = 0; i < ui_->GetElementCount(); ++i) {
        auto* elem = ui_->GetElement(i);
        if (!elem) continue;

        const Vector2 startOffset = center - elem->GetPosition();
        elem->SetShowFunc([startOffset, duration](float elapsed, Vector2& posOffset, Vector4& color) -> bool {
            const float t = std::min(elapsed / duration, 1.f);
            posOffset = Ease::Out::Cubic(startOffset, { 0.f, 0.f }, t);
            color.w  *= t;
            return t >= 1.f;
        });
        elem->PlayShow();
    }
}

