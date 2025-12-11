#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "GameObject/GameObject.hpp"
#include "Command/ICommand.hpp"
#include "Player/Movement/Movement.hpp"
#include "Player/Movement/MovementContext.hpp"
#include <memory>

#include "Collision/Collider.h"
#include "ParticleSystem/ParticleSystem.hpp"

class Enemy : public GameObject {
    struct Status {
        float hp;
        float damage;
    };

    ParticleSystem* particle_ = nullptr;

    std::unique_ptr<Movement> movement_;
    MovementContext movementContext_;

    ICommand* moveCommand_ = nullptr;
    GameObject* target_ = nullptr;

    Status status_ {};

    std::unique_ptr<Collision::Collider> collider_;

    bool invincible_ = false;
    float invincibleTimer_ = 0.4f;
    const float InvincibleDuration = 0.4f;

    bool knockback_ = false;
    float knockbackTimer_ = 0.f;
    const float KnockbackDecay = 0.4f;

    bool shake_ = false;
    uint16_t shakeFrames_ = 0;
    float shakePower_ = 0.f;
    bool shakeDecay_ = true;

    bool scalePulse_ = false;
    float pulseDuration_ = 0.f;
    float pulseTimer_ = 0.f;

    bool dying_ = false;
    float deathTimer_ = 0.f;
    const float DeathDuration = 0.8f;

public:
    void Initialize() override;

    void Update(float deltaTime) override;
    void Draw() override;

    void Debug() override;

    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }

    void SetMoveCommand(ICommand* command) { moveCommand_ = command; }

    Movement* GetMovement() { return movement_.get(); }

    void OnCollision(const Collision::Collider* _collider);

    void SetParticleSystem(ParticleSystem* _particle) { particle_ = _particle; }

    bool IsDead() const { return dying_; }

private:
    void ApplyKnockback(Vector3 _direction, float _force, float _duration);
    void ApplyShake(uint16_t _frames, float _power = 1.f, bool _decay = true);
    void ApplyScalePulse(float _duration);

    void UpdateShake();
    void UpdatePulse(float _deltaTime);
    void UpdateDeath(float _deltaTime);
    void UpdateKnockback(float _deltaTime);
    void UpdateInvincible(float _deltaTime);
    void UpdateMovement(float _deltaTime);
};

#endif // ENEMY_HPP_