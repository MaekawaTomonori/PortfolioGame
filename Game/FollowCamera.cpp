#include "FollowCamera.hpp"

#include "imgui.h"
#include "GameObject/GameObject.hpp"
#include "Json/Json.hpp"
#include "Pattern/Singleton.hpp"

FollowCamera::FollowCamera() : 
    target_(nullptr), 
    input_(nullptr), 
    cameraManager_(nullptr) {
}

FollowCamera::~FollowCamera() {
}

void FollowCamera::Initialize() {
    input_ = Singleton<Input>::GetInstance();
    cameraManager_ = Singleton<CameraController>::GetInstance();

    Load();
    UpdateCameraPosition();
}

void FollowCamera::Update() {
    Debug();

    if (!active_) return;
    if (!target_ || !input_ || !cameraManager_) return;
    
    UpdateCameraPosition();
    ApplyShake();
}

void FollowCamera::SetActive(bool _state) {
    active_ = _state;
}

void FollowCamera::Shake(const float _time, const float _power) {
    shakeTimer_ = _time;
    shakePower_ = _power;
}

void FollowCamera::Load() {
    const auto& json = Singleton<Json>::GetInstance();

    if (json->Load("FollowCamera")){
        offset_ = std::get<Vector3>(json->GetValue("FollowCamera", "Settings", "Offset"));
        yaw_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Yaw"));
        pitch_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Pitch"));
    } else {
        offset_ = {0.0f, 10.0f, 0.0f};
    }
}

void FollowCamera::Save() {
    const auto& json = Singleton<Json>::GetInstance();
    json->SetValue("FollowCamera", "Settings", "Offset", offset_);
    json->SetValue("FollowCamera", "Settings", "Yaw", yaw_);
    json->SetValue("FollowCamera", "Settings", "Pitch", pitch_);
    json->Save("FollowCamera");
}

void FollowCamera::Debug() {
    if (!debug_) return;

    debug_->RegisterCommand("FollowCamera", [this]() {
        ImGui::Begin("FollowCamera");
        ImGui::Text("Active : %s", active_ ? "Active" : "Inactive");
        ImGui::Separator();
        ImGui::Text("Follow Camera Settings");
        ImGui::Text("Camera Offset");
        ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
        ImGui::Text("Yaw : ");
        ImGui::SameLine();
        ImGui::DragFloat("Yaw", &yaw_, 0.1f);
        ImGui::Text("Pitch : ");
        ImGui::SameLine();
        ImGui::DragFloat("Pitch", &pitch_, 0.1f);
        ImGui::Separator();
        if (ImGui::Button("Save Settings")) {
            Save();
        }
        ImGui::End();
    });
}

void FollowCamera::UpdateCameraPosition() const {
    if (!target_ || !cameraManager_) return;

    auto active = cameraManager_->GetActive();
    active->transform_.translate = target_->GetPosition() + offset_;
    active->transform_.rotate = Vector3{pitch_, yaw_, 0.f};
}

void FollowCamera::ApplyShake() {
    if (0.f < shakeTimer_) {
        shake = Vector3::Random() * shakePower_;
    }
    cameraManager_->GetActive()->transform_.translate += shake;
}
