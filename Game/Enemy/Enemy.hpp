#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "GameObject/GameObject.hpp"
#include "Enemy/State/IEnemyState.hpp"
#include "Math/Vector4.hpp"
#include <memory>

#include "Collision/Collider.h"
#include "HpBar/HpBar.hpp"
#include "ParticleSystem/ParticleSystem.hpp"

class IEnemyActionModule;

class Enemy : public GameObject {
    struct Status {
        float hp;
        float damage;
    };

public:
    struct Params {
        float maxHp = 3.0f;
        float moveSpeed   = 3.0f;
        float minDistance = 0.5f;
        float invincibleDuration = 0.4f;
        float knockbackDecay = 0.4f;
        float deathDuration = 0.8f;
    };

private:

    ParticleSystem* particle_ = nullptr;

    GameObject* target_ = nullptr;

    Status status_ {};

    std::unique_ptr<Collision::Collider> collider_;

    // 共通パラメータ（参照）
    const Params* params_ = nullptr;
    Vector3 externalForce_{};

    bool invincible_ = false;
    float invincibleTimer_ = 0.f;

    bool killedByBullet_ = false;

    bool knockback_ = false;
    float knockbackTimer_ = 0.f;

    bool shake_ = false;
    uint16_t shakeFrames_ = 0;
    float shakePower_ = 0.f;
    bool shakeDecay_ = true;

    bool scalePulse_ = false;
    float pulseDuration_ = 0.f;
    float pulseTimer_ = 0.f;

    std::unique_ptr<HpBar> hpBar_;

    std::unique_ptr<IEnemyState> state_;
    std::unique_ptr<IEnemyActionModule> actionModule_;

public:
    /** @brief _params は Enemies が持つものを全員で参照する */
    Enemy(const Params& _params, const Vector3& _position);
    ~Enemy() override;

    void Update() override;
    void Draw() override;

    void Debug() override;

    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }

    void OnCollision(const Collision::Collider* _collider);

    void SetParticleSystem(ParticleSystem* _particle) { particle_ = _particle; }

    bool IsDead() const { return state_ && state_->IsDying(); }
    bool IsKilledByBullet() const { return killedByBullet_; }
    void ForceDeath();

    void AddExternalForce(const Vector3& _force) { externalForce_ += _force; }

    const Params* GetParams() const { return params_; }

    void SetActionModule(std::unique_ptr<IEnemyActionModule> _module);
    IEnemyActionModule* GetActionModule() const { return actionModule_.get(); }

    // 状態クラスからのアクセス用
    bool IsKnockback() const { return knockback_; }
    bool IsInvincible() const { return invincible_; }
    void SetModelColor(const Vector4& _color) const;
    void ApplyShake(uint16_t frames, float power = 1.f, bool decay = true);
    Vector3 ConsumeExternalForce();
    void TransitionState(std::unique_ptr<IEnemyState> _next);

private:
    void ApplyDamage(float _damage, const Vector3& _fromPosition);
    void ApplyKnockback(const Vector3& _direction, float _force, float _duration);
    void ApplyScalePulse(float _duration);

    void UpdateShake();
    void UpdatePulse(float _deltaTime);
    void UpdateKnockback(float _deltaTime);
    void UpdateInvincible(float _deltaTime);
};

#endif // ENEMY_HPP_
