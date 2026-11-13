#define NOMINMAX
#include "StageInfoUI.hpp"

#include <algorithm>
#include <format>
#include <string>

#include "Math/Easing.hpp"
#include "Ui/HudStyle.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr float FADE_DURATION      = 0.5f;
    constexpr float HOLD_DURATION      = 2.0f;
    constexpr float TEXT_FADE_DURATION = 0.2f;
    constexpr float MOVE_DURATION      = 0.6f;

    // 背景の大きさと左上での位置（画面に対する割合）。Assets/UI/StageInfo.json の値と合わせる
    constexpr Vector2 CENTER_SIZE   = {600.f / 1280.f, 170.f / 720.f};
    constexpr Vector2 CORNER_SIZE   = {230.f / 1280.f, 72.f / 720.f};
    constexpr Vector2 CORNER_OFFSET = {(20.f + 230.f * 0.5f) / 1280.f, (16.f + 72.f * 0.5f) / 720.f};
}

void StageInfoUI::TextGroup::SetAlpha(const float _alpha) {
    label.SetColor({HudStyle::GRAY.x, HudStyle::GRAY.y, HudStyle::GRAY.z, HudStyle::GRAY.w * _alpha});
    goal.SetColor({HudStyle::WHITE.x, HudStyle::WHITE.y, HudStyle::WHITE.z, HudStyle::WHITE.w * _alpha});
}

StageInfoUI::~StageInfoUI() {
    ui_.Unload();
}

void StageInfoUI::Initialize() {
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
        large_.SetAlpha(1.f - t);
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
        small_.SetAlpha(t);
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
    large_.SetAlpha(1.f);
    small_.SetAlpha(0.f);
    ui_.SetAlpha(0.f);
    ui_.SetVisible(true);
    NextPhase(Phase::Showing);
}

void StageInfoUI::ShowAtCorner(const uint16_t _requireKill) {
    SetGoalText(_requireKill);

    ApplyMove(1.f);
    large_.SetAlpha(0.f);
    small_.SetAlpha(1.f);
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
