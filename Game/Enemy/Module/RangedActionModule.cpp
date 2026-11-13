#include "RangedActionModule.hpp"

#include "Enemy/Enemy.hpp"
#include "Math/MathUtils.hpp"
#include "Time/Time.hpp"

void RangedActionModule::Update(Enemy* _enemy) {
    if (!params_) return;
    if (_enemy->IsKnockback() || _enemy->IsInvincible()) return;

    const auto* target = _enemy->GetTarget();
    if (!target) return;

    triggerTimer_ += Time().GetDeltaTime();
    if (triggerTimer_ < params_->triggerInterval) return;
    triggerTimer_ = 0.f;

    if (MathUtils::Random(0.f, 1.f) >= params_->triggerChance) return;

    Vector3 toTarget = target->GetPosition() - _enemy->GetPosition();
    toTarget.y = 0.f;
    float distance = toTarget.Length();
    if (distance < params_->minRange || distance > params_->maxRange) return;
    if (distance <= 0.001f) return;

    if (onFire_) {
        onFire_(_enemy, toTarget.Normalize());
    }
}
