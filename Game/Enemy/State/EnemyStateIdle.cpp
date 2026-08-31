#include "EnemyStateIdle.hpp"
#include "Enemy/Enemy.hpp"
#include "Enemy/Module/IEnemyActionModule.hpp"

void EnemyStateIdle::Enter() {
    elapsedTime_ = 0.f;
}

std::unique_ptr<IEnemyState> EnemyStateIdle::Update() {
    const auto* params = enemy_->GetParams();
    if (!params) return nullptr;

    auto* module = enemy_->GetActionModule();
    bool busy = module && module->IsBusy();

    if (!busy && !enemy_->IsKnockback()) {
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
                Vector3 extForce = enemy_->ConsumeExternalForce();
                enemy_->SetVelocity(extForce);
            }
        }
    }

    if (module) module->Update(enemy_);

    return nullptr;
}
