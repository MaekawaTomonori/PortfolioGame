#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "GameObject/GameObject.hpp"
#include "Components/MovementComponent.hpp"
#include "Module/Attack/Attack.hpp"
#include <memory>

#include "Status/PlayerStatus.hpp"

class MovementComponent;

class Player : public GameObject {
    std::unique_ptr<MovementComponent> movement_;
    std::unique_ptr<Attack> attack_;

    std::unique_ptr<Collision::Collider> collider_;

    Vector3 targetPosition_ = {};

    Vector3 forlight_ = {};

    PlayerStatus status_{};

    bool invulnerability_ = false;
    float invulnerabilityTimer_ = 1.f;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;

    void SetTargetPosition(Vector3 _position);

    void OnCollision(const Collision::Collider* _collider);
};
#endif // PLAYER_HPP_