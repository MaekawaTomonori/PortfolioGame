#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "GameObject/GameObject.hpp"
#include "Command/ICommand.hpp"
#include "Player/Movement/Movement.hpp"
#include "Player/Movement/MovementContext.hpp"
#include <memory>

#include "Collision/Collider.h"
#include "HpBar/HpBar.hpp"
#include "ParticleSystem/ParticleSystem.hpp"

class Enemy : public GameObject {
    enum class DashState {
        Idle,       // 通常状態
        Charging,   // ダッシュ準備中
        Dashing     // ダッシュ実行中
    };

    struct Status {
        float hp;
        float damage;
    };

public:
    // 敵の共通パラメータ構造体
    struct Params {
        // 基本ステータス
        float maxHp = 3.0f;

        // 無敵時間
        float invincibleDuration = 0.4f;

        // ノックバック
        float knockbackDecay = 0.4f;

        // 死亡演出
        float deathDuration = 0.8f;

        // ダッシュアクション
        struct {
            float chargeMaxTime = 2.0f;        // チャージ時間
            float duration = 0.4f;              // ダッシュ持続時間
            float speed = 20.0f;                // ダッシュ速度
            float triggerInterval = 3.0f;       // トリガー間隔
            float triggerChance = 0.3f;         // 発動確率（0.0~1.0）
            float enemyOffset = 1.5f;           // 予測線の敵からのオフセット
            float lineLength = 0.7f;            // 予測線の長さ倍率
            float lineWidth = 1.5f;             // 予測線の幅
            float lineCenterOffset = 0.6f;      // 予測線の中心位置オフセット倍率
        } dash;
    };

private:

    ParticleSystem* particle_ = nullptr;

    std::unique_ptr<Movement> movement_;
    MovementContext movementContext_;

    ICommand* moveCommand_ = nullptr;
    GameObject* target_ = nullptr;

    Status status_ {};

    std::unique_ptr<Collision::Collider> collider_;

    // 共通パラメータ（参照）
    const Params* params_ = nullptr;
    Vector3 externalForce_{};

    bool invincible_ = false;
    float invincibleTimer_ = 0.4f;

    bool knockback_ = false;
    float knockbackTimer_ = 0.f;

    bool shake_ = false;
    uint16_t shakeFrames_ = 0;
    float shakePower_ = 0.f;
    bool shakeDecay_ = true;

    bool scalePulse_ = false;
    float pulseDuration_ = 0.f;
    float pulseTimer_ = 0.f;

    bool dying_ = false;
    float deathTimer_ = 0.f;

    // ダッシュアクション関連
    DashState dashState_ = DashState::Idle;
    float dashChargeTime_ = 0.f;
    float dashTimer_ = 0.f;
    Vector3 dashDirection_ {};
    std::unique_ptr<Model> prediction_;
    float dashTriggerTimer_ = 0.f;

    std::unique_ptr<HpBar> hpBar_;

public:
    void Initialize() override;

    void Update(float deltaTime) override;
    void Draw() override;

    void Debug() override;

    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }

    void SetMoveCommand(ICommand* command) { moveCommand_ = command; }

    Movement* GetMovement() const { return movement_.get(); }

    void OnCollision(const Collision::Collider* _collider);

    void SetParticleSystem(ParticleSystem* _particle) { particle_ = _particle; }

    bool IsDead() const { return dying_; }

    void AddExternalForce(const Vector3& _force) { externalForce_ += _force; }

    // パラメータの設定・取得
    void SetParams(const Params* _params) { params_ = _params; }
    const Params* GetParams() const { return params_; }

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

    // ダッシュアクション関連
    void UpdateDashAction(float _deltaTime);
    void TryStartDash();
    void UpdateCharging(float _deltaTime);
    void UpdateDashing(float _deltaTime);
    void UpdatePredictionLine();
};

#endif // ENEMY_HPP_