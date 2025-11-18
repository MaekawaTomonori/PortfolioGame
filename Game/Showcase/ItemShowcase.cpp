#include "ItemShowcase.hpp"

#include "imgui.h"

void ItemShowcase::Update() {
    if (!model_)return;

    if (debugUI_.has_value()) {
        Debug();
    }

    static const float DT = 1.f / 60.f;

    transform_.rotate.y += angle_ * rotateSpd_;
    transform_.rotate.y = fmod(transform_.rotate.y, MathUtils::F_PI * 2.f);

    // sine wave motion
    // t_ : 0 -> 2PI
    // y : -1 -> 1
    t_ += DT;
    t_ = fmod(t_, MathUtils::F_PI * 2.f);
    transform_.translate.y = originalP.y + std::sin(t_) * waveAmp_;

    model_->SetScale(transform_.scale);
    model_->SetRotate(transform_.rotate);
    model_->SetTranslate(transform_.translate);

    model_->Update();
}

void ItemShowcase::Draw() {
    if (!model_)return;

    model_->Draw();
}

ItemShowcase& ItemShowcase::SetPosition(const Vector3& _position) {
    transform_.translate = _position;
    originalP = _position;
    return *this;
}

ItemShowcase& ItemShowcase::SetRotation(const Vector3& _rotation) {
    transform_.rotate = _rotation;
    return *this;
}

ItemShowcase& ItemShowcase::SetScale(const Vector3& _scale) {
    transform_.scale = _scale;
    return *this;
}

ItemShowcase& ItemShowcase::SetRotateSpeed(float _speed) {
    rotateSpd_ = _speed;
    return *this;
}

ItemShowcase& ItemShowcase::SetWaveAmplitude(float _amplitude) {
    waveAmp_ = _amplitude;
    return *this;
}

ItemShowcase& ItemShowcase::SetDebug(const std::reference_wrapper<DebugUI>& _debugUI) {
    debugUI_ = _debugUI;
    return *this;
}

void ItemShowcase::Debug() {
    debugUI_->get().RegisterCommand(
        "ItemShowcase_" + model_->GetUniqueId(), [&]() {
            ImGui::Begin("ItemShowcase");
            ImGui::PushID(model_->GetUniqueId().c_str());
            if (ImGui::CollapsingHeader("ItemShowcase")) {
                ImGui::Text("Position : ");
                ImGui::SameLine();
                ImGui::DragFloat3("##Position", &transform_.translate.x, 0.1f);
                ImGui::Text("Rotation : ");
                ImGui::SameLine();
                ImGui::DragFloat3("##Rotation", &transform_.rotate.x, 0.1f);
                ImGui::Text("Scale : ");
                ImGui::SameLine();
                ImGui::DragFloat3("##Scale", &transform_.scale.x, 0.1f);
                ImGui::Text("Rotate Speed Mul : ");
                ImGui::SameLine();
                ImGui::DragFloat("##RotateSpeedMul", &rotateSpd_, 0.1f, 0.f, 10.f);
                ImGui::Text("Wave Height Mul : ");
                ImGui::SameLine();
                ImGui::DragFloat("##WaveHeightMul", &waveAmp_, 0.1f, 0.f, 10.f);
                if (ImGui::Button("Reset Transform")) {
                    transform_ = Transform{
                        {1, 1, 1},
                        Vector3{0, 0, 0},
                        {0, 0, 0},
                    };
                    originalP = { 0,0,0 };
                    rotateSpd_ = 1.f;
                    waveAmp_ = 1.f;
                }
            }
            ImGui::PopID();
            ImGui::End();
        }
    );
}
