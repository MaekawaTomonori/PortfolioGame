#include "Enemy.hpp"

#include "ColliderType.hpp"

void Enemy::Initialize() {
    SetModel("animatedcube");
    model_->SetTexture("white_x16.png");
    model_->SetColor({1.f, 0.3f, 0.3f, 1.f});

    status_ = {
        .hp = 100.f,
        .damage = 0.f
    };

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _pCol){ this->OnCollision(_pCol); })
        ->SetOwner(this)
        ->SetType(Collision::Type::AABB)
        ->SetSize(Collision::Vec3{1.f, 1.f, 1.f})
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->AddAttribute(static_cast<uint32_t>(CollisionType::Enemy))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::Enemy))
        ->Enable();
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;

    // 移動コマンドを実行（velocityを設定）
    // if(!UpdateKnockback()){
    if (knockback_) {
        velocity_ *= KnockbackDecay;
        knockbackTimer_ -= deltaTime;

        if (knockbackTimer_ <= 0.f) {
            knockback_ = false;
            knockbackTimer_ = 0.f;
        }
    } else {
        if (moveCommand_ && target_) {
            moveCommand_->Execute(this, target_);
        }

        if (invincible_) {
            velocity_ *= 0.3f;
        }
    }

    // UpdateInvincible()
    if (invincible_) {
        invincibleTimer_ -= deltaTime;

        // 進行度を計算 (1.0 → 0.0)
        float progress = invincibleTimer_ / InvincibleDuration;

        // 白(2.0, 2.0, 2.0) → 赤(1.0, 0.3, 0.3) にグラデーション
        float r = MathUtils::Lerp(1.0f, 2.0f, progress);
        float g = MathUtils::Lerp(0.3f, 2.0f, progress);
        float b = MathUtils::Lerp(0.3f, 2.0f, progress);
        model_->SetColor({r, g, b, 1.f});

        if (invincibleTimer_ <= 0.f) {
            invincible_ = false;
            invincibleTimer_ = InvincibleDuration;
            model_->SetColor({1.f, 0.3f, 0.3f, 1.f});
        }
    }

    UpdateShake();
    UpdatePulse(deltaTime);

    // velocityを位置に適用
    ApplyVelocity(deltaTime);

    collider_->SetTranslate({position_.x, position_.y, position_.z});

    UpdateModel();
}

void Enemy::Draw() {
    if (!active_ || !model_) return;

    model_->Draw();
}

void Enemy::OnCollision(const Collision::Collider* _collider) {
    if (invincible_) return;
    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::Player)) {
        active_ = false;
        collider_->Disable();
        return;
    }

    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::P_Bullet)) {
        // Damage()
        {
            // Damage処理
            --status_.hp;
            
            // 無敵状態に移行
            invincible_ = true;

            // 強烈に白く光る（2.0倍の明るさ）
            model_->SetColor({2.f, 2.f, 2.f, 1.f});

            // Knockback処理
            Collision::Vec3 d = (collider_->GetTranslate() - _collider->GetTranslate());
            ApplyKnockback({d.x, d.y, d.z}, 40.f, 0.15f);

            // Shake
            ApplyShake(10, 0.5f);

            // ScalePulse
            ApplyScalePulse(0.15f);

            // Particle
            if (particle_) {
                Vector3 p = position_;
                p.y += 1.5f;
                particle_->Edit("hit").SetPosition(p).Emit();
            }

            if (status_.hp <= 0.f) {
                active_ = false;
                collider_->Disable();
            }
        }
    }
}

void Enemy::ApplyKnockback(Vector3 _direction, float _force, float _duration) {
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

    // 減衰処理
    shakePower_ = shakeDecay_ ? shakePower_ * 0.9f : shakePower_;
    --shakeFrames_;
}

void Enemy::UpdatePulse(float _deltaTime) {
    if (!scalePulse_) return;

    pulseTimer_ += _deltaTime;

    if (pulseTimer_ >= pulseDuration_) {
        // 終了
        scalePulse_ = false;
        pulseTimer_ = 0.f;
        scale_ = Vector3{1.f, 1.f, 1.f};
        return;
    }

    // 進行度 (0.0 → 1.0)
    float progress = pulseTimer_ / pulseDuration_;

    // sin波で滑らかに (0 → 1 → 0)
    constexpr float PI = 3.14159265f;
    float curve = std::sin(progress * PI);

    // スケール計算 (1.0 → 1.2 → 1.0)
    float scaleFactor = 1.0f + (curve * 0.2f);
    scale_ = Vector3{scaleFactor, scaleFactor, scaleFactor};
}
