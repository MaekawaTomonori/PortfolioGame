#include "EnemyStateIdle.hpp"
#include "EnemyStateCharging.hpp"
#include "Enemy/Enemy.hpp"
#include "Math/MathUtils.hpp"

void EnemyStateIdle::Enter() {
    elapsedTime_ = 0.f;
    triggerTimer_ = 0.f;
}

std::unique_ptr<IEnemyState> EnemyStateIdle::Update(float _deltaTime) {
    const auto* params = enemy_->GetParams();
    if (!params) return nullptr;

    // ノックバック中は移動しない
    if (enemy_->IsKnockback()) {
        return nullptr;
    }

    // ターゲット方向への移動
    const auto* target = enemy_->GetTarget();
    if (target) {
        Vector3 toTarget = target->GetPosition() - enemy_->GetPosition();
        toTarget.y = 0.f;

        float distance = toTarget.Length();
        if (distance > params->minDistance) {
            Vector3 velocity = toTarget.Normalize() * params->moveSpeed;

            if (enemy_->IsInvincible()) {
                velocity *= 0.3f;
            }

            velocity += enemy_->ConsumeExternalForce();
            enemy_->SetVelocity(velocity);
        } else {
            // minDistance 以内なら停止
            Vector3 extForce = enemy_->ConsumeExternalForce();
            enemy_->SetVelocity(extForce);
        }
    }

    // ダッシュトリガー判定
    if (!enemy_->IsKnockback() && !enemy_->IsInvincible() && target) {
        triggerTimer_ += _deltaTime;
        if (triggerTimer_ >= params->dash.triggerInterval) {
            triggerTimer_ = 0.f;
            if (MathUtils::Random(0.f, 1.f) < params->dash.triggerChance) {
                Vector3 toTarget = target->GetPosition() - enemy_->GetPosition();
                toTarget.y = 0.f;
                if (toTarget.Length() > 0.001f) {
                    return std::make_unique<EnemyStateCharging>(enemy_, toTarget.Normalize());
                }
            }
        }
    }

    return nullptr;
}
