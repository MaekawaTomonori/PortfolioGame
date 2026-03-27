#include "EnemyStateDeath.hpp"
#include "Enemy/Enemy.hpp"
#include "Math/MathUtils.hpp"

void EnemyStateDeath::Enter() {
    enemy_->SetVelocity({ 0.f, 0.f, 0.f });
    elapsedTime_ = 0.f;
}

std::unique_ptr<IEnemyState> EnemyStateDeath::Update(float deltaTime) {
    const auto* params = enemy_->GetParams();
    if (!params) {
        enemy_->SetActive(false);
        return nullptr;
    }

    elapsedTime_ += deltaTime;

    if (elapsedTime_ >= params->deathDuration) {
        enemy_->SetActive(false);
        return nullptr;
    }

    float progress = elapsedTime_ / params->deathDuration;

    if (progress < 0.3f) {
        // フェーズ1: 膨張
        float phase = progress / 0.3f;
        float scaleValue = MathUtils::Lerp(0.5f, 0.65f, phase);
        enemy_->SetScale({ scaleValue, scaleValue, scaleValue });
        enemy_->SetModelColor({ 2.f, 2.f, 2.f, 1.f });

        Vector3 rot = enemy_->GetRotation();
        rot.y += deltaTime * 10.0f * (1.0f + phase * 2.0f);
        enemy_->SetRotation(rot);
    } else {
        // フェーズ2: 縮小＆フェードアウト
        float phase = (progress - 0.3f) / 0.5f;
        float scaleValue = MathUtils::Lerp(0.75f, 0.0f, phase);
        enemy_->SetScale({ scaleValue, scaleValue, scaleValue });
        enemy_->SetModelColor({ 2.f, 2.f, 2.f, 1.0f - phase });

        Vector3 rot = enemy_->GetRotation();
        rot.y += deltaTime * 20.0f;
        enemy_->SetRotation(rot);

        Vector3 pos = enemy_->GetPosition();
        pos.y += deltaTime * 2.0f;
        enemy_->SetPosition(pos);
    }

    return nullptr;
}
