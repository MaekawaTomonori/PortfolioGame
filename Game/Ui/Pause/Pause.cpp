#include "Pause.hpp"

#include <algorithm>

#include "src/Time/Time.hpp"

namespace {
    constexpr float FADE_TIME = 0.2f;
}

Pause::~Pause() {
    ui_.Unload();
}

void Pause::Initialize() {
    ui_ = GEUI::Canvas::Load("Assets/UI/Pause.json");
    ui_.SetVisible(false);

    resume_ = ui_.Find("Resume");
    retry_ = ui_.Find("Retry");
    quit_ = ui_.Find("Quit");
}

void Pause::Update() {
    CheckActivateState();

    if (!IsOpen()) return;

    HandleInput();
    UpdateFade(Time().GetDeltaTime());
}

void Pause::UpdateFade(const float _deltaTime) {
    fadeTime_ = (std::min)(fadeTime_ + _deltaTime, FADE_TIME);
    const float t = fadeTime_ / FADE_TIME;
    ui_.SetAlpha(1.f - (1.f - t) * (1.f - t));
}

void Pause::HandleInput() {
    if (const auto direction = menuInput_.GetDirection()) ui_.Navigate(*direction);
    if (menuInput_.IsSubmit()) ui_.Submit();

    // Retry / Quit はシーンを切り替えるので、押されたものだけを呼ぶ
    if (resume_.WasPressed()) {
        Close();
    } else if (retry_.WasPressed()) {
        if (onRetry_) onRetry_();
    } else if (quit_.WasPressed()) {
        if (onQuit_) onQuit_();
    }
}

void Pause::Open() {
    // 薄い状態から出して、UpdateFade で濃くしていく
    fadeTime_ = 0.f;
    ui_.SetAlpha(0.f);
    ui_.SetVisible(true);
    resume_.SetSelected(true);
}

void Pause::Close() {
    ui_.SetVisible(false);
    if (onResume_) onResume_();
}

bool Pause::IsOpen() const {
    return ui_.IsVisible();
}

void Pause::SetOnResume(std::function<void()> _callback) {
    onResume_ = std::move(_callback);
}

void Pause::SetOnRetry(std::function<void()> _callback) {
    onRetry_ = std::move(_callback);
}

void Pause::SetOnQuit(std::function<void()> _callback) {
    onQuit_ = std::move(_callback);
}

void Pause::CheckActivateState() {
    if (menuInput_.IsCancel()) {
        IsOpen() ? Close() : Open();
    }
}
