#ifndef BULLET_HPP_
#define BULLET_HPP_
#include "GameObject/GameObject.hpp"
#include "Collision/Collider.h"
#include "CollisionAttribute.hpp"
#include "Combat/DamageSource.hpp"

class Bullet : public GameObject{
public:
    struct Params {
        float speed = 5.f;
        float lifetime = 2.f;
        float damage = 1.f;
    };

private:
    GameObject* owner_ = nullptr;

    float speed_ = 0.f;
    float lifetime_ = 2.f;
    float piercing_ = 0.f;

    CollisionAttribute attribute_ = CollisionAttribute::PlayerBullet;
    CollisionAttribute ignoreAttribute_ = CollisionAttribute::Player;
    CollisionAttribute targetAttribute_ = CollisionAttribute::Enemy;

    std::unique_ptr<Collision::Collider> collider_;

    DamageSource damageSource_;

public:
    void Initialize(GameObject* _owner, const Vector3& _direction, const Params& _params = {},
        CollisionAttribute _attribute = CollisionAttribute::PlayerBullet, CollisionAttribute _ignoreAttribute = CollisionAttribute::Player,
        CollisionAttribute _targetAttribute = CollisionAttribute::Enemy);
    void Update() override;
    void Draw() override;

private:
    void Initialize() override;
    void OnCollision(const Collision::Collider* _collider);
}; // class Bullet

#endif // BULLET_HPP_
