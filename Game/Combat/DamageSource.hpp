#ifndef DAMAGE_SOURCE_HPP_
#define DAMAGE_SOURCE_HPP_

namespace Collision {
    class Collider;
}

class DamageSource {
    float damage_ = 0.f;

public:
    void SetDamage(float _damage);
    float GetDamage() const;

    void AttachTo(Collision::Collider& _collider);

    [[nodiscard]] static float GetDamage(const Collision::Collider& _collider);
};

#endif // DAMAGE_SOURCE_HPP_
