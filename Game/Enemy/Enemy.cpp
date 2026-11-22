#include "Enemy.hpp"
#include "Enemy/Module/IEnemyActionModule.hpp"
#include "Enemy/State/EnemyStateIdle.hpp"
#include "Enemy/State/EnemyStateDeath.hpp"

#include "CollisionAttribute.hpp"
#include "Combat/DamageSource.hpp"
#include "Math/MathUtils.hpp"
#include "Time/Time.hpp"
#include "Utils.hpp"
#include "imgui.h"

namespace {
    // 外観
    constexpr Vector4 HIT_FLASH_COLOR  = {2.f, 2.f,  2.f,  1.f};   // 被弾フラッシュ白

    // コライダー
    constexpr Vector3 COLLIDER_SIZE    = {1.f, 1.f, 1.f};

    // 被弾時エフェクト
    constexpr float    HIT_KNOCKBACK_FORCE    = 40.f;
    constexpr float    HIT_KNOCKBACK_DURATION = 0.15f;
    constexpr uint16_t HIT_SHAKE_FRAMES       = 10;
    constexpr float    HIT_SHAKE_POWER        = 0.5f;
    constexpr float    HIT_PULSE_DURATION     = 0.15f;

    // パーティクル
    constexpr float PARTICLE_Y_OFFSET = 1.5f;

    // シェイク
    constexpr float SHAKE_DECAY_FACTOR = 0.9f;

    // スケール
    constexpr float   PULSE_AMPLITUDE  = 0.2f;           // スケールパルスの振れ幅係数
}

Enemy::Enemy(const Params& _params, const Vector3& _position)
    : params_(&_params) {
    position_ = _position;
    scale_ = DEFAULT_SCALE;

    SetModel("animatedcube");
    model_->SetName("Enemy");
    model_->SetTexture("white_x16.png");
    model_->SetColor(BASE_COLOR);

    status_ = {
        .hp = params_->maxHp,
        .damage = 0.f
    };

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _pCol){ this->OnCollision(_pCol); })
        ->SetOwner(this)
        ->SetType(Collision::Type::AABB)
        ->SetSize(COLLIDER_SIZE)
        ->SetTranslate(position_)
        ->AddAttribute(static_cast<uint32_t>(CollisionAttribute::Enemy))
        ->AddIgnore(static_cast<uint32_t>(CollisionAttribute::Enemy))
        ->AddIgnore(static_cast<uint32_t>(CollisionAttribute::EnemyBullet))
        ->Enable();

    hpBar_ = std::make_unique<HpBar>();
    hpBar_->Initialize(&position_);

    state_ = std::make_unique<EnemyStateIdle>(this);
    state_->Enter();
}

Enemy::~Enemy() = default;

void Enemy::SetActionModule(std::unique_ptr<IEnemyActionModule> _module) {
    actionModule_ = std::move(_module);
}

void Enemy::Update() {
    if (!active_) return;

    float deltaTime = Time().GetDeltaTime();

    if (knockback_) UpdateKnockback(deltaTime);

    if (state_) {
        if (auto next = state_->Update()) TransitionState(std::move(next));
    }

    UpdateInvincible(deltaTime);
    UpdateShake();
    UpdatePulse(deltaTime);

    ApplyVelocity();
    collider_->SetTranslate(position_);
    hpBar_->Update(status_.hp / params_->maxHp);
    UpdateModel();
}

void Enemy::Draw() {
    if (!active_ || !model_) return;
    model_->Draw();
    if (state_) state_->Draw();
    if (actionModule_) actionModule_->Draw();
    hpBar_->Draw();
}

void Enemy::Debug() {
#ifdef _DEBUG
    if (!ImGui::TreeNode("Enemy Info")) return;

    ImGui::Text("State: %s", state_ ? state_->GetStateName() : "None");
    ImGui::Text("HP: %.1f / %.1f", status_.hp, params_->maxHp);
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", position_.x, position_.y, position_.z);
    ImGui::Text("Speed: %.2f", velocity_.Length());
    ImGui::Text("Invincible: %s / Knockback: %s", invincible_ ? "Yes" : "No", knockback_ ? "Yes" : "No");

    ImGui::TreePop();
#endif
}

void Enemy::OnCollision(const Collision::Collider* _collider) {
    if (invincible_) return;
    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionAttribute::Player)) {
        active_ = false;
        collider_->Disable();

        // Particle
        if (particle_) {
            Vector3 p = position_;
            p.y += PARTICLE_Y_OFFSET;
            particle_->Emit("enemy_hit", p);
        }

        return;
    }

    if (const float damage = DamageSource::GetDamage(*_collider); damage > 0.f) {
        ApplyDamage(damage, _collider->GetTranslate());
    }
}

void Enemy::ApplyDamage(const float _damage, const Vector3& _fromPosition) {
    status_.hp -= _damage;

    invincible_ = true;
    invincibleTimer_ = params_->invincibleDuration;

    model_->SetColor(HIT_FLASH_COLOR);

    ApplyKnockback(collider_->GetTranslate() - _fromPosition, HIT_KNOCKBACK_FORCE, HIT_KNOCKBACK_DURATION);

    ApplyShake(HIT_SHAKE_FRAMES, HIT_SHAKE_POWER);
    ApplyScalePulse(HIT_PULSE_DURATION);

    if (particle_) {
        Vector3 p = position_;
        p.y += PARTICLE_Y_OFFSET;
        particle_->Emit("enemy_hit", p);
    }

    if (status_.hp <= 0.f) {
        killedByBullet_ = true;
        collider_->Disable();
        TransitionState(std::make_unique<EnemyStateDeath>(this));
    }
}

void Enemy::ApplyKnockback(const Vector3& _direction, float _force, float _duration) {
    if (knockback_) return;

    knockback_ = true;
    knockbackTimer_ = _duration;

    velocity_ = _direction.Normalize() * _force;
}

void Enemy::ApplyShake(const uint16_t _frames, const float _power, const bool _decay) {
    shake_ = true;
    shakeFrames_ = _frames;
    shakePower_ = _power;
    shakeDecay_ = _decay;
}

void Enemy::ApplyScalePulse(const float _duration) {
    scalePulse_ = true;
    pulseDuration_ = _duration;
    pulseTimer_ = 0.f;
}

void Enemy::UpdateShake() {
    if (!shake_) return;

    if (shakeFrames_ <= 0) {
        shake_ = false;
        shakeFrames_ = 0;
        return;
    }

    // ランダムなオフセットを生成（描画用のみ）
    Vector3 offset = Vector3::Random() * shakePower_;
    offset.y = 0.f;
    offset_ = offset;

    shakePower_ = shakeDecay_ ? shakePower_ * SHAKE_DECAY_FACTOR : shakePower_;
    --shakeFrames_;
}

void Enemy::UpdatePulse(float _deltaTime) {
    if (!scalePulse_) return;

    pulseTimer_ += _deltaTime;

    if (pulseTimer_ >= pulseDuration_) {
        scalePulse_ = false;
        pulseTimer_ = 0.f;
        scale_ = DEFAULT_SCALE;
        return;
    }

    // 半周期の sin にすると、一度膨らんでから元の大きさに戻る
    float progress = pulseTimer_ / pulseDuration_;
    float curve = std::sin(progress * MathUtils::F_PI);
    float scaleFactor = DEFAULT_SCALE.x * (1.0f + curve * PULSE_AMPLITUDE);
    scale_ = Vector3{scaleFactor, scaleFactor, scaleFactor};
}

void Enemy::UpdateKnockback(float _deltaTime) {
    velocity_ *= params_->knockbackDecay;
    knockbackTimer_ -= _deltaTime;

    if (knockbackTimer_ <= 0.f) {
        knockback_ = false;
        knockbackTimer_ = 0.f;
    }
}

void Enemy::UpdateInvincible(float _deltaTime) {
    if (!invincible_) return;

    invincibleTimer_ -= _deltaTime;

    // 無敵が切れるにつれて、被弾の白から通常の色へ戻す
    const float remaining = invincibleTimer_ / params_->invincibleDuration;
    model_->SetColor(MathUtils::Lerp(BASE_COLOR, HIT_FLASH_COLOR, remaining));

    if (invincibleTimer_ <= 0.f) {
        invincible_ = false;
        invincibleTimer_ = 0.f;
        model_->SetColor(BASE_COLOR);
    }
}

void Enemy::SetModelColor(const Vector4& _color) const {
    if (model_) model_->SetColor(_color);
}

Vector3 Enemy::ConsumeExternalForce() {
    Vector3 f = externalForce_;
    externalForce_ = {};
    return f;
}

void Enemy::TransitionState(std::unique_ptr<IEnemyState> _next) {
    if (state_) state_->Exit();
    state_ = std::move(_next);
    if (state_) state_->Enter();
}

void Enemy::ForceDeath() {
    if (IsDead()) return;
    if (collider_) collider_->Disable();
    TransitionState(std::make_unique<EnemyStateDeath>(this));
}
