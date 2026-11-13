#define NOMINMAX
#include "KillCounter.hpp"

#include <algorithm>
#include <format>

#include "imgui.h"
#include "Enemy/Enemies.hpp"
#include "Ui/HudLayout.hpp"
#include "Ui/HudStyle.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr const char* CANVAS_PATH = "Assets/UI/KillCounter.json";
}

KillCounter::~KillCounter() {
    ui_.Unload();
}

void KillCounter::Initialize() {
    ui_ = GEUI::Canvas::Load(CANVAS_PATH);
    ui_.SetVisible(false);

    root_ = ui_.Find("Counter");
    text_ = root_.Find("Text");
    fill_ = root_.Find("Bar/Fill");
}

void KillCounter::Update() {
    if (!enemies_ || !ui_.IsVisible()) return;

    fadeElapsed_ = std::min(fadeElapsed_ + Time().GetDeltaTime(), HudStyle::FADE_DURATION);
    ui_.SetAlpha(fadeElapsed_ / HudStyle::FADE_DURATION);

    const uint16_t count = std::min(max_, enemies_->GetDeathCount());
    if (count != shown_) {
        shown_ = count;
        ApplyCount(count);
    }
}

void KillCounter::Debug() {
#ifdef _DEBUG
    ImGui::Begin("KillCounter");
    ImGui::Text("Count: %u / %u", static_cast<unsigned>(shown_), static_cast<unsigned>(max_));
    if (ImGui::Button("Play Intro")) PlayIntroAnim();

    HudLayout::Group groups[] = {{"Counter", root_}};
    for (HudLayout::Group& group : groups) {
        HudLayout::DrawEditor(group);
    }
    HudLayout::DrawSaveButton(CANVAS_PATH, groups);
    ImGui::End();
#endif
}

void KillCounter::Setup(Enemies* _enemies, const uint16_t _max) {
    enemies_ = _enemies;
    max_ = _max;
    shown_ = 0;
    ApplyCount(0);
}

void KillCounter::SetActive(const bool _active) {
    ui_.SetVisible(_active);
}

void KillCounter::PlayIntroAnim() {
    fadeElapsed_ = 0.f;
    ui_.SetAlpha(0.f);
    ui_.SetVisible(true);
}

void KillCounter::ApplyCount(const uint16_t _count) {
    const bool reached = max_ > 0 && _count >= max_;
    text_.SetText(std::format("KILL: {}/{}{}", _count, max_, reached ? " OK!" : ""));
    text_.SetColor(reached ? HudStyle::YELLOW : HudStyle::WHITE);

    const float ratio = max_ > 0 ? static_cast<float>(_count) / static_cast<float>(max_) : 0.f;
    fill_.SetSize({ratio, 1.f});
}
