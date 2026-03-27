#include "EnemyStateDash.hpp"
#include "EnemyStateIdle.hpp"
#include "Enemy/Enemy.hpp"

EnemyStateDash::EnemyStateDash(Enemy* _enemy, const Vector3& _dashDirection)
    : IEnemyState(_enemy), dashDirection_(_dashDirection) {}

void EnemyStateDash::Enter() {
    elapsedTime_ = 0.f;
    enemy_->ApplyShake(8, 0.5f, true);
    enemy_->SetModelColor({ 2.f, 2.f, 2.f, 1.f });
}

std::unique_ptr<IEnemyState> EnemyStateDash::Update(const float _deltaTime) {
    const auto* params = enemy_->GetParams();
    if (!params) return std::make_unique<EnemyStateIdle>(enemy_);

    elapsedTime_ += _deltaTime;

    enemy_->SetVelocity(dashDirection_ * params->dash.speed);

    if (elapsedTime_ >= params->dash.duration) {
        return std::make_unique<EnemyStateIdle>(enemy_);
    }

    return nullptr;
}

void EnemyStateDash::Exit() {
    enemy_->SetVelocity({ 0.f, 0.f, 0.f });
    enemy_->SetModelColor({ 1.f, 0.3f, 0.3f, 1.f });
    enemy_->SetScale({ 0.5f, 0.5f, 0.5f });
}
