#ifndef BULLET_HPP_
#define BULLET_HPP_
#include <cstdint>
#include "Skill/SkillEntity.hpp"
#include "CollisionAttribute.hpp"
#include "Combat/DamageSource.hpp"

class Bullet : public SkillEntity {
public:
    struct Params {
        float speed = 5.f;
        float lifetime = 2.f;
        float damage = 1.f;
    };

private:
    Params params_;
    int32_t piercing_ = 0;

    CollisionAttribute attribute_ = CollisionAttribute::PlayerBullet;
    CollisionAttribute ignoreAttribute_ = CollisionAttribute::Player;
    CollisionAttribute targetAttribute_ = CollisionAttribute::Enemy;

    DamageSource damageSource_;

public:
    /** @brief Initialize より前に呼ぶ。撃った時点の値をコピーして持つ */
    void SetParams(const Params& _params);

    void Initialize(const Vector3& _position, const Vector3& _direction) override;
    void Update() override;
    void Draw() override;

private:
    void OnCollision(const Collision::Collider* _collider);
}; // class Bullet

#endif // BULLET_HPP_
