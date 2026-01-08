#include "GameTimer.hpp"

#include <algorithm>

#include "imgui_internal.h"

void GameTimer::Initialize() {
    sprites_[0] = std::make_unique<Sprite>();
    sprites_[0]->Initialize("numbers.png");
    sprites_[0]->SetSize(size_);
    sprites_[0]->SetPosition({position_.x - 32.f, position_.y});
    sprites_[0]->SetAnchorPoint({ 0.5f, 0.5f });
    sprites_[0]->SetColor({ 1.f, 1.f, 1.f, 1.f });
    sprites_[0]->SetTextureLeftTop({ 0.f, 0.f });
    sprites_[0]->SetTextureSize(TEXTURE_SIZE);

    sprites_[1] = std::make_unique<Sprite>();
    sprites_[1]->Initialize("numbers.png");
    sprites_[1]->SetSize(size_);
    sprites_[1]->SetPosition({position_.x + 32.f, position_.y});
    sprites_[1]->SetAnchorPoint({ 0.5f, 0.5f });
    sprites_[1]->SetColor({ 1.f, 1.f, 1.f, 1.f });
    sprites_[1]->SetTextureLeftTop({ 0.f, 0.f });
    sprites_[1]->SetTextureSize(TEXTURE_SIZE);
}

void GameTimer::Update(const float _deltaTime) {
    time_ -= _deltaTime;
    done_ = false;

    if (time_ <= 0.f) {
        time_ = 0.f;
        done_ = true;
    }

    int i = std::clamp(static_cast<int>(time_), 0, 99);

    for (int j = 1; j >= 0; --j) {
        sprites_[j]->SetPosition({ position_.x + (j - 0.5f) * size_.x, position_.y });
        sprites_[j]->SetSize(size_);
        int v = (i % 10);
        sprites_[j]->SetTextureLeftTop({ v * TEXTURE_SIZE.x, 0.f });
        sprites_[j]->Update();
        i /= 10;
    }
}

void GameTimer::Draw() const {
    for (const auto& sprite : sprites_) {
        sprite->Draw();
    }
}

void GameTimer::Debug() {
#ifdef _DEBUG
    ImGui::Begin("Game Timer Debug");

    ImGui::Text("Time");
    ImGui::SetNextItemWidth(150.f);
    ImGui::DragFloat("##Time", &time_, 0.1f, 0.f, 100.f);

    ImGui::Text("Position");
    ImGui::SetNextItemWidth(150.f);
    ImGui::DragFloat2("##Position", &position_.x, 1.f);

    ImGui::Text("Size");
    ImGui::SetNextItemWidth(150.f);
    ImGui::DragFloat2("##Size", &size_.x, 1.f);

    ImGui::End();
#endif
}

void GameTimer::SetDuration(const float _sec) {
    time_ = _sec;
}
