#ifndef Bullet_HPP_
#define Bullet_HPP_
#include "GameObject/GameObject.hpp"
#include "Collision/Collider.h"

class Bullet : public GameObject{
public:
    struct Params {
        float speed = 5.f;
        float lifetime = 2.f;
    };

private:
    GameObject* owner_ = nullptr;

    float speed_ = 0.f;
    float lifetime_ = 2.f;
    float piercing_ = 0.f;

    std::unique_ptr<Collision::Collider> collider_;

public:
    void Initialize(GameObject* _owner, const Vector3& _direction, const Params& _params = {});
    void Update(float deltaTime) override;
    void Draw() override;

private:
    void Initialize() override;
    void OnCollision(const Collision::Collider* _collider);
}; // class Bullet

#endif // Bullet_HPP_
