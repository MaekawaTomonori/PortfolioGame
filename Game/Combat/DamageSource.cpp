#include "DamageSource.hpp"
#include "Collision/Collider.h"
#include "CollisionAttribute.hpp"

void DamageSource::SetDamage(const float _damage) {
    damage_ = _damage < 0.f ? 0.f : _damage;
}

float DamageSource::GetDamage() const {
    return damage_;
}

void DamageSource::AttachTo(Collision::Collider& _collider) {
    _collider.SetOwner(this);
    _collider.AddAttribute(static_cast<uint32_t>(CollisionAttribute::Damage));
}

float DamageSource::GetDamage(const Collision::Collider& _collider) {
    // Damage 属性は AttachTo でしか付かない
    if (!(_collider.GetAttribute() & static_cast<uint32_t>(CollisionAttribute::Damage))) return 0.f;

    auto* source = static_cast<const DamageSource*>(_collider.GetOwner());
    if (!source) return 0.f;

    return source->GetDamage();
}
