#ifndef Bullet_HPP_
#define Bullet_HPP_
#include "GameObject/GameObject.hpp"
#include "Collision/Collider.h"
#include "ColliderType.hpp"

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
    float damage_ = 1.f;
    float piercing_ = 0.f;

    CollisionType attribute_ = CollisionType::P_Bullet;
    CollisionType ignoreAttribute_ = CollisionType::Player;
    CollisionType targetAttribute_ = CollisionType::Enemy;

    std::unique_ptr<Collision::Collider> collider_;

public:
    void Initialize(GameObject* _owner, const Vector3& _direction, const Params& _params = {},
        CollisionType _attribute = CollisionType::P_Bullet, CollisionType _ignoreAttribute = CollisionType::Player,
        CollisionType _targetAttribute = CollisionType::Enemy);
    void Update() override;
    void Draw() override;

    float GetDamage() const { return damage_; }

private:
    void Initialize() override;
    void OnCollision(const Collision::Collider* _collider);
}; // class Bullet

#endif // Bullet_HPP_
