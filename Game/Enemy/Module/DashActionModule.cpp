#include "DashActionModule.hpp"

#include <cmath>
#include "Enemy/Enemy.hpp"
#include "Model.hpp"
#include "Math/MathUtils.hpp"
#include "Math/Vector4.hpp"
#include "Time/Time.hpp"

namespace {
    constexpr Vector4 IDLE_COLOR = {1.f, 0.3f, 0.3f, 1.f};
    constexpr Vector3 IDLE_SCALE = {0.5f, 0.5f, 0.5f};
}

void DashActionModule::Update(Enemy* _enemy) {
    if (!params_) return;

    switch (phase_) {
    case Phase::Waiting:
        UpdateWaiting(_enemy);
        break;
    case Phase::Charging:
        UpdateCharging(_enemy);
        break;
    case Phase::Dashing:
        UpdateDashing(_enemy);
        break;
    }
}

void DashActionModule::Draw() {
    if (prediction_) {
        prediction_->Draw();
    }
}

void DashActionModule::UpdateWaiting(Enemy* _enemy) {
    if (_enemy->IsKnockback() || _enemy->IsInvincible()) return;

    const auto* target = _enemy->GetTarget();
    if (!target) return;

    triggerTimer_ += Time().GetDeltaTime();
    if (triggerTimer_ < params_->triggerInterval) return;
    triggerTimer_ = 0.f;

    if (MathUtils::Random(0.f, 1.f) >= params_->triggerChance) return;

    Vector3 toTarget = target->GetPosition() - _enemy->GetPosition();
    toTarget.y = 0.f;
    if (toTarget.Length() <= 0.001f) return;

    dashDirection_ = toTarget.Normalize();
    EnterCharging(_enemy);
}

void DashActionModule::EnterCharging(Enemy* _enemy) {
    phase_ = Phase::Charging;
    elapsedTime_ = 0.f;

    prediction_ = std::make_unique<Model>();
    prediction_->Initialize("plane");
    prediction_->SetTexture("white_x16.png");

    float dashDistance = params_->speed * params_->duration;
    float yaw = atan2f(dashDirection_.x, dashDirection_.z);
    float lineLength = dashDistance * params_->lineLength;

    prediction_->SetScale({ params_->lineWidth, lineLength, 1.f });
    prediction_->SetRotate({ -MathUtils::F_PI / 2.f, yaw, 0.f });

    Vector3 startPos = _enemy->GetPosition() + dashDirection_ * params_->enemyOffset;
    Vector3 centerPos = startPos + dashDirection_ * (lineLength * params_->lineCenterOffset);
    centerPos.y = 0.1f;
    prediction_->SetTranslate(centerPos);

    _enemy->ApplyShake(5, 0.3f, false);
}

void DashActionModule::UpdateCharging(Enemy* _enemy) {
    elapsedTime_ += Time().GetDeltaTime();
    _enemy->SetVelocity({0.f, 0.f, 0.f});

    float progress = elapsedTime_ / params_->chargeMaxTime;

    float r = MathUtils::Lerp(1.0f, 1.5f, progress);
    float g = MathUtils::Lerp(0.3f, 1.5f, progress);
    float b = MathUtils::Lerp(0.3f, 0.3f, progress);
    _enemy->SetModelColor({ r, g, b, 1.f });

    float pulse = std::sin(elapsedTime_ * 10.0f) * 0.025f * progress;
    float scaleValue = 0.5f + pulse;
    _enemy->SetScale({ scaleValue, scaleValue, scaleValue });

    UpdatePredictionLine();

    if (elapsedTime_ >= params_->chargeMaxTime) {
        prediction_.reset();
        EnterDashing(_enemy);
    }
}

void DashActionModule::UpdatePredictionLine() {
    if (!prediction_) return;

    float progress = elapsedTime_ / params_->chargeMaxTime;

    float r = MathUtils::Lerp(1.0f, 1.5f, progress);
    float g = MathUtils::Lerp(1.0f, 0.3f, progress);
    float b = MathUtils::Lerp(0.3f, 0.3f, progress);
    prediction_->SetColor({ r, g, b, 0.6f });

    float dashDistance = params_->speed * params_->duration;
    float lineLength = dashDistance * params_->lineLength;
    float pulseScale = 1.0f + std::sin(elapsedTime_ * 15.0f) * 0.1f * progress;
    prediction_->SetScale({ params_->lineWidth * pulseScale, lineLength, 1.0f });

    prediction_->Update();
}

void DashActionModule::EnterDashing(Enemy* _enemy) {
    phase_ = Phase::Dashing;
    elapsedTime_ = 0.f;
    _enemy->ApplyShake(8, 0.5f, true);
    _enemy->SetModelColor({ 2.f, 2.f, 2.f, 1.f });
}

void DashActionModule::UpdateDashing(Enemy* _enemy) {
    elapsedTime_ += Time().GetDeltaTime();
    _enemy->SetVelocity(dashDirection_ * params_->speed);

    if (elapsedTime_ >= params_->duration) {
        _enemy->SetVelocity({ 0.f, 0.f, 0.f });
        _enemy->SetModelColor(IDLE_COLOR);
        _enemy->SetScale(IDLE_SCALE);
        phase_ = Phase::Waiting;
    }
}
