#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "GameObject/GameObject.hpp"
#include "Movement/Movement.hpp"
#include "Movement/IMovementBehavior.hpp"
#include "Movement/MovementContext.hpp"
#include "Module/Attack/Attack.hpp"
#include <memory>
#include <vector>

#include "FollowCamera.hpp"
#include "Status/PlayerStatus.hpp"
#include "InputHandler.hpp"
#include "ParticleSystem/ParticleSystem.hpp"

class Player : public GameObject {
    std::unique_ptr<Movement> movement_;
    std::unique_ptr<Attack> attack_;

    std::unique_ptr<InputHandler> inputHandler_;

    std::vector<std::unique_ptr<IMovementBehavior>> behaviors_;

    std::unique_ptr<Collision::Collider> collider_;

    MovementContext movementContext_;

    bool targetExist_ = false;
    Vector3 targetPosition_ = {};

    Vector3 forlight_ = {};

    PlayerStatus status_{};

    bool invulnerability_ = false;
    float invulnerabilityTimer_ = 1.f;
    const float InvulnerabilityDuration = 1.f;

    FollowCamera* camera_ = nullptr;

    ParticleSystem* particle_ = nullptr;

    // プレイヤーの基本色
    const Vector4 BaseColor = {0.3f, 0.3f, 1.f, 1.f};
    const Vector4 DamageFlashColor = {2.f, 2.f, 2.f, 1.f};

# ifdef _DEBUG
//#define NO_ATK 
//#define NO_MOVE
#endif


public:
    void Initialize(ParticleSystem* _particle);
    void Update(float deltaTime) override;
    void Draw() override;
    void Debug() const override;

    void SetTargetPosition(Vector3 _position);

    void OnCollision(const Collision::Collider* _collider);

    void SetCamera(FollowCamera* _camera);

private:
    void Initialize() override;
    void UpdateInvulnerability(float deltaTime);
};
#endif // PLAYER_HPP_