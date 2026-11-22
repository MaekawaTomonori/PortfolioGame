#include "GameTimer.hpp"

#include <algorithm>
#include <cmath>
#include <format>

#include "imgui.h"
#include "Ui/HudLayout.hpp"
#include "Ui/HudStyle.hpp"
#include "Ui/UiTheme.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr const char* CANVAS_PATH = "Assets/UI/Timer.json";

    // 残り時間がこの割合を切ったらゲージを赤くする
    constexpr float LOW_TIME_RATIO = 0.25f;
}

GameTimer::~GameTimer() {
    ui_.Unload();
}

void GameTimer::Initialize(const UiTheme& _theme) {
    theme_ = &_theme;

    ui_ = GEUI::Canvas::Load(CANVAS_PATH);
    ui_.SetVisible(false);

    root_ = ui_.Find("Timer");
    text_ = root_.Find("Text");
    fill_ = root_.Find("Bar/Fill");
}

void GameTimer::Update() {
    if (!ui_.IsVisible()) return;

    const float deltaTime = Time().GetDeltaTime();

    // フェードインが終わるまでは時間を進めない
    if (fadeElapsed_ < HudStyle::FADE_DURATION) {
        fadeElapsed_ = std::min(fadeElapsed_ + deltaTime, HudStyle::FADE_DURATION);
        ui_.SetAlpha(fadeElapsed_ / HudStyle::FADE_DURATION);
    } else if (!done_ && !paused_) {
        time_ = std::max(0.f, time_ - deltaTime);
        done_ = time_ <= 0.f;
    }

    ApplyTime();
}

void GameTimer::Debug() {
#ifdef _DEBUG
    ImGui::Begin("Game Timer Debug");
    // 残り時間を直接動かす。0 にするとそのままラウンド終了の判定が通る
    if (ImGui::SliderFloat("Remaining", &time_, 0.f, duration_, "%.2f s")) {
        done_ = time_ <= 0.f;
        ApplyTime();
    }
    ImGui::Checkbox("Stop", &paused_);
    ImGui::Text("Duration: %.2f", duration_);
    ImGui::Text("Done: %s", done_ ? "Yes" : "No");
    if (ImGui::Button("Reset")) SetDuration(duration_);
    ImGui::SameLine();
    if (ImGui::Button("Play Intro")) PlayIntroAnim();

    HudLayout::Group groups[] = {{"Timer", root_}};
    for (HudLayout::Group& group : groups) {
        HudLayout::DrawEditor(group);
    }
    HudLayout::DrawSaveButton(CANVAS_PATH, groups);
    ImGui::End();
#endif
}

void GameTimer::SetDuration(const float _sec) {
    duration_ = _sec;
    time_ = _sec;
    done_ = false;
    ApplyTime();
}

void GameTimer::SetActive(const bool _active) {
    ui_.SetVisible(_active);
}

void GameTimer::PlayIntroAnim() {
    fadeElapsed_ = 0.f;
    ui_.SetAlpha(0.f);
    ui_.SetVisible(true);
}

void GameTimer::ApplyTime() {
    // 切り上げて表示し、開始時に設定値、終了時に 0 が見えるようにする
    const int seconds = std::max(0, static_cast<int>(std::ceil(time_)));
    text_.SetText(std::format("TIME: {}", seconds));

    const float ratio = duration_ > 0.f ? time_ / duration_ : 0.f;
    fill_.SetSize({ratio, 1.f});
    fill_.SetColor(theme_->Get(ratio > LOW_TIME_RATIO ? UiColor::Accent : UiColor::Danger));
}
