#define NOMINMAX
#include "FollowCamera.hpp"

#include <algorithm>
#include "imgui.h"
#include "GameObject/GameObject.hpp"
#include "Enemy/Enemies.hpp"
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr Vector3 INITIAL_CAMERA_POSITION = {0.f, 4.1f, -7.6f};
    constexpr Vector3 INITIAL_CAMERA_ROTATION = {0.89f, 0.f, 0.f};

    constexpr float IDLE_ZOOM_LERP_SPEED  = 0.05f;
    constexpr float DEFAULT_FOV           = 0.45f;
    constexpr float MIN_PITCH_COS         = 0.65f;
    constexpr float ENEMY_DISTANCE_WEIGHT = 0.3f;
    constexpr float ZOOM_IN_LERP_SPEED    = 0.1f;
    constexpr float ZOOM_OUT_LERP_SPEED   = 0.03f;
    constexpr float SHAKE_DECAY_RATE      = 0.9f;
}

FollowCamera::FollowCamera() :
    cameraController_(nullptr) {
}

FollowCamera::~FollowCamera() {
}

void FollowCamera::Initialize() {
    cameraController_ = Singleton<CameraController>::GetInstance();
    cameraController_->GetActive()->transform_.translate = INITIAL_CAMERA_POSITION;
    cameraController_->GetActive()->transform_.rotate = INITIAL_CAMERA_ROTATION;

    Load();
    UpdateCameraDistance();
    UpdateCameraPosition();
}

void FollowCamera::Update() {
    if (!active_) return;
    if (!cameraController_) return;

    UpdateCameraDistance();
    UpdateCameraPosition();
    ApplyShake();
}

void FollowCamera::SetActive(const bool _state) {
    active_ = _state;
}

void FollowCamera::Shake(const float _time, const float _power) {
    shakeTimer_ = _time;
    shakePower_ = _power;
    shaking_ = true;
}

void FollowCamera::Load() {
    const auto& json = Singleton<JsonParams>::GetInstance();

    if (json->Load("FollowCamera")){
        distance_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Distance"));
        yaw_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Yaw"));
        pitch_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "Pitch"));
        minDistance_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "MinDistance"));
        maxDistance_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "MaxDistance"));
        zoomSensitivity_ = std::get<float>(json->GetValue("FollowCamera", "Settings", "ZoomSensitivity"));
    }
}

void FollowCamera::Save() {
    const auto& json = Singleton<JsonParams>::GetInstance();
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
        distance_ = MathUtils::Lerp(distance_, minDistance_, IDLE_ZOOM_LERP_SPEED);
        return;
    }

    float maxEnemyDistance = enemies_->GetFarthestEnemyDistance(target_->GetPosition());

    float targetDistance = minDistance_;

    if (0.f < maxEnemyDistance) {
        auto camera = cameraController_->GetActive();
        float fov = DEFAULT_FOV;
        if (camera) {
            fov = camera->GetFov();
        }

        float halfFov = fov * 0.5f;
        float verticalViewDistance = maxEnemyDistance / tanf(halfFov);
        float pitchFactor = 1.0f / std::max(MIN_PITCH_COS, cosf(pitch_));

        targetDistance = (verticalViewDistance * pitchFactor + maxEnemyDistance * ENEMY_DISTANCE_WEIGHT) * zoomSensitivity_;
    }

    targetDistance = std::clamp(targetDistance, minDistance_, maxDistance_);

    float lerpSpeed = (targetDistance > distance_) ? ZOOM_IN_LERP_SPEED : ZOOM_OUT_LERP_SPEED;
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
    ImGui::DragFloat("##Pitch", &pitch_, 0.01f, -MathUtils::F_PI/2.f, MathUtils::F_PI/2.f);
    ImGui::Text("Interpolation Time");
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("##InterpolationTime", &interpolationTime_, 0.01f, 0.0f, 1.0f);
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
    if (!cameraController_) return;
    if (!target_) return;

    auto& active = cameraController_->GetActive()->transform_;

    float cosPitch = cosf(pitch_ + MathUtils::F_PI/2.f);
    float sinPitch = sinf(pitch_ + MathUtils::F_PI/2.f);
    float cosYaw = cosf(yaw_);
    float sinYaw = sinf(yaw_);

    offset_ = Vector3{
        sinYaw * sinPitch,
        -cosPitch,
        -cosYaw * sinPitch
    };

    Vector3 destination = target_->GetPosition() + offset_ * distance_;
    active.translate = MathUtils::Lerp(active.translate, destination, interpolationTime_);
    active.rotate = Vector3{ pitch_, yaw_, 0.f };
}

void FollowCamera::ApplyShake() {
    if (!shaking_) return;

    if (shakeTimer_ < 0.f) {
        shaking_ = false;
        shakeTimer_ = 0.f;
        shake = {0.f, 0.f, 0.f};
    }

    shakeTimer_ -= Time::GetDeltaTime();
    shake = Vector3::Random() * shakePower_;
    cameraController_->GetActive()->transform_.translate += shake;
    shakePower_ *= SHAKE_DECAY_RATE;
}
