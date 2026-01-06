#define NOMINMAX
#include "FollowCamera.hpp"

#include <algorithm>
#include "imgui.h"
#include "GameObject/GameObject.hpp"
#include "Enemy/Enemies.hpp"
#include "Json/Json.hpp"
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"

FollowCamera::FollowCamera() : 
    cameraManager_(nullptr) {
}

FollowCamera::~FollowCamera() {
}

void FollowCamera::Initialize() {
    cameraManager_ = Singleton<CameraController>::GetInstance();

    Load();
    UpdateCameraDistance();
    UpdateCameraPosition();
}

void FollowCamera::Update() {
    if (!active_) return;
    if (!cameraManager_) return;

    UpdateCameraDistance();
    UpdateCameraPosition();
    ApplyShake();
}

void FollowCamera::SetActive(bool _state) {
    active_ = _state;
}

void FollowCamera::Shake(const float _time, const float _power) {
    shakeTimer_ = _time;
    shakePower_ = _power;
    shaking_ = true;
}

void FollowCamera::Load() {
    const auto& json = Singleton<Json>::GetInstance();

    if (json->Load("FollowCamera")){
        distance_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Distance"));
        yaw_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Yaw"));
        pitch_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Pitch"));
        minDistance_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "MinDistance"));
        maxDistance_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "MaxDistance"));
        zoomSensitivity_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "ZoomSensitivity"));
    } else {
        distance_ = 15.0f;
        yaw_ = 0.0f;
        pitch_ = -0.785f;
    }
}

void FollowCamera::Save() {
    const auto& json = Singleton<Json>::GetInstance();
    json->SetValue("FollowCamera", "Settings", "Distance", distance_);
    json->SetValue("FollowCamera", "Settings", "Yaw", yaw_);
    json->SetValue("FollowCamera", "Settings", "Pitch", pitch_);
    json->SetValue("FollowCamera", "Settings", "MinDistance", minDistance_);
    json->SetValue("FollowCamera", "Settings", "MaxDistance", maxDistance_);
    json->SetValue("FollowCamera", "Settings", "ZoomSensitivity", zoomSensitivity_);
    json->Save("FollowCamera");
}

void FollowCamera::UpdateCameraDistance() {
    if (!enemies_ || !target_) {
        distance_ = MathUtils::Lerp(distance_, minDistance_, 0.05f);
        return;
    }

    float maxEnemyDistance = enemies_->GetFarthestEnemyDistance(target_->GetPosition());

    float targetDistance = minDistance_;

    if (0.f < maxEnemyDistance) {
        auto* camera = cameraManager_->GetActive();
        float fov = 0.45f;
        if (camera) {
            fov = camera->GetFov();
        }

        float halfFov = fov * 0.5f;
        float verticalViewDistance = maxEnemyDistance / tanf(halfFov);
        float pitchFactor = 1.0f / std::max(0.65f, cosf(pitch_));

        targetDistance = (verticalViewDistance * pitchFactor + maxEnemyDistance * 0.3f) * zoomSensitivity_;
    }

    targetDistance = std::clamp(targetDistance, minDistance_, maxDistance_);

    float lerpSpeed = (targetDistance > distance_) ? 0.1f : 0.03f;
    distance_ = MathUtils::Lerp(distance_, targetDistance, lerpSpeed);
}

void FollowCamera::Debug() {
    ImGui::Begin("FollowCamera");
    ImGui::Text("Active : %s", active_ ? "Active" : "Inactive");

    ImGui::Separator();

    ImGui::Text("Camera Offset");
    ImGui::DragFloat3("Offset", &offset_.x, 0.1f, -50.0f, 50.0f);

    ImGui::Separator();

    ImGui::Text("Camera Settings (Angle & Distance)");
    ImGui::Text("Distance");
    ImGui::DragFloat("##Distance", &distance_, 0.1f, 1.0f, 100.0f);
    ImGui::Text("Angle (Yaw, Pitch)");
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("##Yaw", &yaw_, 0.01f, -MathUtils::F_PI/2.f, MathUtils::F_PI/2.f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("##Pitch", &pitch_, 0.01f, -MathUtils::F_PI/2.f, MathUtils::F_PI/2.f);

    ImGui::Separator();

    ImGui::Text("Dynamic Zoom Settings");
    ImGui::Text("Distance Range (Min, Max)");
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("##Min Distance", &minDistance_, 0.1f, 1.0f, maxDistance_ - 1.f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("##Max Distance", &maxDistance_, 0.1f, minDistance_ + 1.f, 100.0f);

    ImGui::DragFloat("Zoom Sensitivity", &zoomSensitivity_, 0.01f, 0.1f, 2.0f);

    ImGui::Separator();
    if (target_) {
        Vector3 pos = target_->GetPosition();
        ImGui::Text("Target Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Target Set");
    }

    ImGui::Separator();
    if (ImGui::Button("Save Settings")) {
        Save();
    }
    ImGui::End();
}

void FollowCamera::UpdateCameraPosition() {
    if (!cameraManager_) return;
    if (!target_) return;

    auto active = cameraManager_->GetActive();

    float cp = cosf(pitch_ + MathUtils::F_PI/2.f);
    float sp = sinf(pitch_ + MathUtils::F_PI/2.f);
    float cy = cosf(yaw_);
    float sy = sinf(yaw_);

    offset_ = Vector3{
        sy * sp,
        -cp,
        -cy * sp
    };

    active->transform_.translate = target_->GetPosition() + offset_ * distance_;
    active->transform_.rotate = Vector3{ pitch_, yaw_, 0.f };
}

void FollowCamera::ApplyShake() {
    if (!shaking_) return;

    if (shakeTimer_ < 0.f) {
        shaking_ = false;
        shakeTimer_ = 0.f;
        shake = {0.f, 0.f, 0.f};
    }

    shakeTimer_ -= 1.f / 60.f;
    shake = Vector3::Random() * shakePower_;
    cameraManager_->GetActive()->transform_.translate += shake;
    shakePower_ *= 0.9f;
}
