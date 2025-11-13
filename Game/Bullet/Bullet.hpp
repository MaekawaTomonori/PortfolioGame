#ifndef Bullet_HPP_
#define Bullet_HPP_
#include "GameObject/GameObject.hpp"
#include "Collision/Collider.h"

class Bullet : public GameObject{
    GameObject* owner_ = nullptr;

    float speed_ = 0.f;
    float lifetime_ = 2.f;
    float piercing_ = 0.f;

    std::unique_ptr<Collision::Collider> collider_;

public:
    void Initialize(GameObject* _owner, Vector3 _direction);
    void Update(float deltaTime) override;
    void Draw() override;

private:
    void Initialize() override;
    void OnCollision(const Collision::Collider* _collider);
}; // class Bullet

#endif // Bullet_HPP_
