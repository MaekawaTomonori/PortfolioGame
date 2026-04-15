#include "EnemyStateCharging.hpp"
#include "EnemyStateDash.hpp"
#include "Enemy/Enemy.hpp"
#include "Model.hpp"
#include "Math/MathUtils.hpp"

EnemyStateCharging::EnemyStateCharging(Enemy* enemy, const Vector3& dashDirection)
    : IEnemyState(enemy), dashDirection_(dashDirection) {}

void EnemyStateCharging::Enter() {
    elapsedTime_ = 0.f;

    const auto* params = enemy_->GetParams();
    if (!params) return;

    prediction_ = std::make_unique<Model>();
    prediction_->Initialize("plane");
    prediction_->SetTexture("white_x16.png");

    float dashDistance = params->dash.speed * params->dash.duration;
    float yaw = atan2f(dashDirection_.x, dashDirection_.z);
    float lineLength = dashDistance * params->dash.lineLength;

    prediction_->SetScale({ params->dash.lineWidth, lineLength, 1.f });
    prediction_->SetRotate({ -MathUtils::F_PI / 2.f, yaw, 0.f });

    Vector3 startPos = enemy_->GetPosition() + dashDirection_ * params->dash.enemyOffset;
    Vector3 centerPos = startPos + dashDirection_ * (lineLength * params->dash.lineCenterOffset);
    centerPos.y = 0.1f;
    prediction_->SetTranslate(centerPos);

    enemy_->ApplyShake(5, 0.3f, false);
}

std::unique_ptr<IEnemyState> EnemyStateCharging::Update(float deltaTime) {
    const auto* params = enemy_->GetParams();
    if (!params) return nullptr;

    elapsedTime_ += deltaTime;

    enemy_->SetVelocity({ 0.f, 0.f, 0.f });

    float progress = elapsedTime_ / params->dash.chargeMaxTime;

    // 色: 赤 → 黄色
    float r = MathUtils::Lerp(1.0f, 1.5f, progress);
    float g = MathUtils::Lerp(0.3f, 1.5f, progress);
    float b = MathUtils::Lerp(0.3f, 0.3f, progress);
    enemy_->SetModelColor({ r, g, b, 1.f });

    // スケールパルス（溜めている感じ）
    float pulse = std::sin(elapsedTime_ * 10.0f) * 0.025f * progress;
    float scaleValue = 0.5f + pulse;
    enemy_->SetScale({ scaleValue, scaleValue, scaleValue });

    UpdatePredictionLine();

    if (elapsedTime_ >= params->dash.chargeMaxTime) {
        return std::make_unique<EnemyStateDash>(enemy_, dashDirection_);
    }

    return nullptr;
}

void EnemyStateCharging::Exit() {
    prediction_.reset();
}

void EnemyStateCharging::Draw() {
    if (prediction_) {
        prediction_->Draw();
    }
}

void EnemyStateCharging::UpdatePredictionLine() {
    if (!prediction_) return;
    const auto* params = enemy_->GetParams();
    if (!params) return;

    float progress = elapsedTime_ / params->dash.chargeMaxTime;

    float r = MathUtils::Lerp(1.0f, 1.5f, progress);
    float g = MathUtils::Lerp(1.0f, 0.3f, progress);
    float b = MathUtils::Lerp(0.3f, 0.3f, progress);
    prediction_->SetColor({ r, g, b, 0.6f });

    float dashDistance = params->dash.speed * params->dash.duration;
    float lineLength = dashDistance * params->dash.lineLength;
    float pulseScale = 1.0f + std::sin(elapsedTime_ * 15.0f) * 0.1f * progress;
    prediction_->SetScale({ params->dash.lineWidth * pulseScale, lineLength, 1.0f });

    prediction_->Update();
}
