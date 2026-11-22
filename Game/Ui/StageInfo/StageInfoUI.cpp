#include "StageInfoUI.hpp"

#include <algorithm>
#include <format>
#include <string>

#include "Math/Easing.hpp"
#include "Ui/HudStyle.hpp"
#include "Ui/UiTheme.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr float FADE_DURATION      = 0.5f;
    constexpr float HOLD_DURATION      = 2.0f;
    constexpr float TEXT_FADE_DURATION = 0.2f;
    constexpr float MOVE_DURATION      = 0.6f;

    // 背景の大きさと左上での位置（画面に対する割合）。Assets/UI/StageInfo.json の値と合わせる
    constexpr Vector2 CENTER_SIZE   = {600.f / HudStyle::REFERENCE_SIZE.x, 170.f / HudStyle::REFERENCE_SIZE.y};
    constexpr Vector2 CORNER_SIZE   = {230.f / HudStyle::REFERENCE_SIZE.x, 72.f / HudStyle::REFERENCE_SIZE.y};
    constexpr Vector2 CORNER_OFFSET = {(20.f + 230.f * 0.5f) / HudStyle::REFERENCE_SIZE.x, (16.f + 72.f * 0.5f) / HudStyle::REFERENCE_SIZE.y};
}

void StageInfoUI::TextGroup::SetAlpha(const float _alpha, const UiTheme& _theme) {
    const Vector4& labelColor = _theme.Get(UiColor::TextDim);
    const Vector4& goalColor = _theme.Get(UiColor::Text);
    label.SetColor({labelColor.x, labelColor.y, labelColor.z, labelColor.w * _alpha});
    goal.SetColor({goalColor.x, goalColor.y, goalColor.z, goalColor.w * _alpha});
}

StageInfoUI::~StageInfoUI() {
    ui_.Unload();
}

void StageInfoUI::Initialize(const UiTheme& _theme) {
    theme_ = &_theme;

    ui_ = GEUI::Canvas::Load("Assets/UI/StageInfo.json");
    ui_.SetVisible(false);

    panel_ = ui_.Find("Panel");
    large_ = {panel_.Find("Large/Label"), panel_.Find("Large/Goal")};
    small_ = {panel_.Find("Small/Label"), panel_.Find("Small/Goal")};
    centerOffset_ = panel_.GetOffset();
}

void StageInfoUI::Update() {
    if (phase_ == Phase::Done) return;

    elapsed_ += Time().GetDeltaTime();

    switch (phase_) {
    case Phase::Showing: {
        const float t = std::min(elapsed_ / FADE_DURATION, 1.f);
        ui_.SetAlpha(t);
        if (t >= 1.f) NextPhase(Phase::Hold);
        break;
    }
    case Phase::Hold:
        if (elapsed_ >= HOLD_DURATION) NextPhase(Phase::HideLargeText);
        break;

    case Phase::HideLargeText: {
        const float t = std::min(elapsed_ / TEXT_FADE_DURATION, 1.f);
        large_.SetAlpha(1.f - t, *theme_);
        if (t >= 1.f) {
            NextPhase(Phase::Moving);
            if (onMoveStart_) onMoveStart_();
        }
        break;
    }
    case Phase::Moving: {
        const float t = std::min(elapsed_ / MOVE_DURATION, 1.f);
        ApplyMove(t);
        if (t >= 1.f) NextPhase(Phase::ShowSmallText);
        break;
    }
    case Phase::ShowSmallText: {
        const float t = std::min(elapsed_ / TEXT_FADE_DURATION, 1.f);
        small_.SetAlpha(t, *theme_);
        if (t >= 1.f) NextPhase(Phase::Done);
        break;
    }
    case Phase::Done:
        break;
    }
}

void StageInfoUI::Setup(const uint16_t _requireKill) {
    SetGoalText(_requireKill);

    ApplyMove(0.f);
    large_.SetAlpha(1.f, *theme_);
    small_.SetAlpha(0.f, *theme_);
    ui_.SetAlpha(0.f);
    ui_.SetVisible(true);
    NextPhase(Phase::Showing);
}

void StageInfoUI::ShowAtCorner(const uint16_t _requireKill) {
    SetGoalText(_requireKill);

    ApplyMove(1.f);
    large_.SetAlpha(0.f, *theme_);
    small_.SetAlpha(1.f, *theme_);
    ui_.SetAlpha(1.f);
    ui_.SetVisible(true);
    NextPhase(Phase::Done);
}

void StageInfoUI::SetActive(const bool _active) {
    ui_.SetVisible(_active);
}

void StageInfoUI::SetOnMoveStart(std::function<void()> _callback) {
    onMoveStart_ = std::move(_callback);
}

void StageInfoUI::SetGoalText(const uint16_t _requireKill) {
    const std::string goal = std::format("Defeat {} enemies", _requireKill);
    large_.goal.SetText(goal);
    small_.goal.SetText(goal);
}

void StageInfoUI::ApplyMove(const float _progress) {
    // 同じ背景を、位置と大きさを合わせて左上の配置へ動かす
    panel_.SetOffset(Ease::Out::Cubic(centerOffset_, CORNER_OFFSET, _progress));
    panel_.SetSize(Ease::Out::Cubic(CENTER_SIZE, CORNER_SIZE, _progress));
}

void StageInfoUI::NextPhase(const Phase _phase) {
    phase_   = _phase;
    elapsed_ = 0.f;
}
