#include "Enemy.hpp"
#include "Enemy/State/EnemyStateIdle.hpp"
#include "Enemy/State/EnemyStateDeath.hpp"

#include "ColliderType.hpp"
#include "Math/MathUtils.hpp"
#include "imgui.h"

namespace {
    // 外観
    constexpr Vector4 BASE_COLOR       = {1.f, 0.3f, 0.3f, 1.f};   // 通常時の赤
    constexpr Vector4 HIT_FLASH_COLOR  = {2.f, 2.f,  2.f,  1.f};   // 被弾フラッシュ白

    // コライダー
    constexpr Vector3 COLLIDER_SIZE    = {1.f, 1.f, 1.f};

    // HPバーの色
    constexpr Vector4 HPBAR_FRONT_COLOR = {1.f, 0.f, 0.f, 1.f};
    constexpr Vector4 HPBAR_BACK_COLOR  = {0.f, 0.f, 0.f, 0.6f};

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
    constexpr Vector3 DEFAULT_SCALE    = {0.5f, 0.5f, 0.5f};
    constexpr float   PULSE_AMPLITUDE  = 0.2f;           // スケールパルスの振れ幅係数

    // カラーグラデーション（無敵状態：白→赤）
    constexpr float GRAD_R_BASE  = 1.0f;
    constexpr float GRAD_G_BASE  = 0.3f;
    constexpr float GRAD_B_BASE  = 0.3f;
    constexpr float GRAD_FLASH   = 2.0f;

    // Debugウィンドウ
    constexpr float DEBUG_KNOCKBACK_DURATION = HIT_KNOCKBACK_DURATION;
}

void Enemy::Initialize() {
    SetModel("animatedcube");
    model_->SetName("Enemy");
    model_->SetTexture("white_x16.png");
    model_->SetColor(BASE_COLOR);

    status_ = {
        .hp = params_ ? params_->maxHp : 3.0f,
        .damage = 0.f
    };

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _pCol){ this->OnCollision(_pCol); })
        ->SetOwner(this)
        ->SetType(Collision::Type::AABB)
        ->SetSize(COLLIDER_SIZE)
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->AddAttribute(static_cast<uint32_t>(CollisionType::Enemy))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::Enemy))
        ->Enable();

    hpBar_ = std::make_unique<HpBar>();
    hpBar_->Initialize("white_x16.png", &position_);
    hpBar_->SetFrontColor(HPBAR_FRONT_COLOR);
    hpBar_->SetBackColor(HPBAR_BACK_COLOR);

    state_ = std::make_unique<EnemyStateIdle>(this);
    state_->Enter();
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;

    if (knockback_) UpdateKnockback(deltaTime);

    if (state_) {
        if (auto next = state_->Update(deltaTime)) TransitionState(std::move(next));
    }

    UpdateInvincible(deltaTime);
    UpdateShake();
    UpdatePulse(deltaTime);

    ApplyVelocity(deltaTime);
    collider_->SetTranslate({position_.x, position_.y, position_.z});
    hpBar_->Update(status_.hp / params_->maxHp);
    UpdateModel();
}

void Enemy::Draw() {
    if (!active_ || !model_) return;
    model_->Draw();
    if (state_) state_->Draw();
    hpBar_->Draw();
}

void Enemy::Debug() {
#ifdef _DEBUG
    if (ImGui::TreeNode("Enemy Info")) {
        // 基本情報
        ImGui::Text("Active: %s", active_ ? "Yes" : "No");

        ImGui::Separator();

        // ステータス
        if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("HP: %.1f", status_.hp);
            ImGui::ProgressBar(status_.hp / (params_ ? params_->maxHp : 3.0f), ImVec2(-1, 0), "");
            ImGui::Text("Damage: %.1f", status_.damage);
        }

        // 位置情報
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", position_.x, position_.y, position_.z);
            ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", rotation_.x, rotation_.y, rotation_.z);
            ImGui::Text("Scale: (%.2f, %.2f, %.2f)", scale_.x, scale_.y, scale_.z);
            ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);
            ImGui::Text("Velocity Length: %.2f", velocity_.Length());
        }

        // Collider情報
        if (ImGui::CollapsingHeader("Collider")) {
            if (collider_) {
                auto pos = collider_->GetTranslate();
                auto size = get<Vector3>(collider_->GetSize());
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
                ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);
                ImGui::Text("Enabled: %s", collider_->IsEnabled() ? "Yes" : "No");

                // Position と Collider のズレをチェック
                float diff = std::sqrt(
                    (position_.x - pos.x) * (position_.x - pos.x) +
                    (position_.y - pos.y) * (position_.y - pos.y) +
                    (position_.z - pos.z) * (position_.z - pos.z)
                );
                ImGui::TextColored(
                    diff > 0.01f ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                    "Position Mismatch: %.4f", diff
                );
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Collider");
            }
        }

        // 状態フラグ
        if (ImGui::CollapsingHeader("State Flags")) {
            ImGui::Checkbox("Invincible", (bool*)&invincible_);
            if (invincible_ && params_) {
                ImGui::SameLine();
                ImGui::Text("(%.2fs)", invincibleTimer_);
                ImGui::ProgressBar(invincibleTimer_ / params_->invincibleDuration, ImVec2(-1, 0));
            }

            ImGui::Checkbox("Knockback", (bool*)&knockback_);
            if (knockback_) {
                ImGui::SameLine();
                ImGui::Text("(%.2fs)", knockbackTimer_);
                ImGui::ProgressBar(knockbackTimer_ / DEBUG_KNOCKBACK_DURATION, ImVec2(-1, 0));
            }

            ImGui::Checkbox("Shake", (bool*)&shake_);
            if (shake_) {
                ImGui::SameLine();
                ImGui::Text("(%d frames, power: %.2f)", shakeFrames_, shakePower_);
            }

            ImGui::Checkbox("Scale Pulse", (bool*)&scalePulse_);
            if (scalePulse_) {
                ImGui::SameLine();
                ImGui::Text("(%.2fs / %.2fs)", pulseTimer_, pulseDuration_);
                ImGui::ProgressBar(pulseTimer_ / pulseDuration_, ImVec2(-1, 0));
            }
        }

        // ターゲット情報
        if (ImGui::CollapsingHeader("Target & Movement")) {
            if (target_) {
                Vector3 targetPos = target_->GetPosition();
                ImGui::Text("Target: Present");
                ImGui::Text("Target Pos: (%.2f, %.2f, %.2f)", targetPos.x, targetPos.y, targetPos.z);

                Vector3 toTarget = targetPos - position_;
                float distance = toTarget.Length();
                ImGui::Text("Distance: %.2f", distance);

                if (distance > 0.01f) {
                    Vector3 direction = toTarget.Normalize();
                    ImGui::Text("Direction: (%.2f, %.2f, %.2f)", direction.x, direction.y, direction.z);
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Target: None");
            }
        }

        if (ImGui::CollapsingHeader("State", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Current: %s", state_ ? state_->GetStateName() : "None");
        }

        ImGui::TreePop();
    }
#endif
}

void Enemy::OnCollision(const Collision::Collider* _collider) {
    if (invincible_) return;
    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::Player)) {
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

    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::P_Bullet)) {
        // Damage()
        {
            // Damage処理
            --status_.hp;

            // 無敵状態に移行
            invincible_ = true;

            // 強烈に白く光る
            model_->SetColor(HIT_FLASH_COLOR);

            // Knockback処理
            Vector3 d = (collider_->GetTranslate() - _collider->GetTranslate());
            ApplyKnockback({d.x, d.y, d.z}, HIT_KNOCKBACK_FORCE, HIT_KNOCKBACK_DURATION);

            // Shake
            ApplyShake(HIT_SHAKE_FRAMES, HIT_SHAKE_POWER);

            // ScalePulse
            ApplyScalePulse(HIT_PULSE_DURATION);

            // Particle
            if (particle_) {
                Vector3 p = position_;
                p.y += PARTICLE_Y_OFFSET;
                particle_->Emit("enemy_hit", p);
            }

            if (status_.hp <= 0.f) {
                collider_->Disable();
                TransitionState(std::make_unique<EnemyStateDeath>(this));
            }
        }
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

    // 減衰処理
    shakePower_ = shakeDecay_ ? shakePower_ * SHAKE_DECAY_FACTOR : shakePower_;
    --shakeFrames_;
}

void Enemy::UpdatePulse(float _deltaTime) {
    if (!scalePulse_) return;

    pulseTimer_ += _deltaTime;

    if (pulseTimer_ >= pulseDuration_) {
        // 終了
        scalePulse_ = false;
        pulseTimer_ = 0.f;
        scale_ = DEFAULT_SCALE;
        return;
    }

    // 進行度 (0.0 → 1.0)
    float progress = pulseTimer_ / pulseDuration_;

    // sin波で滑らかに (0 → 1 → 0)
    float curve = std::sin(progress * MathUtils::F_PI);

    // スケール計算 (DEFAULT_SCALE → 振れ幅 → DEFAULT_SCALE)
    float scaleFactor = DEFAULT_SCALE.x * (1.0f + curve * PULSE_AMPLITUDE);
    scale_ = Vector3{scaleFactor, scaleFactor, scaleFactor};
}

void Enemy::UpdateKnockback(float _deltaTime) {
    if (params_) {
        velocity_ *= params_->knockbackDecay;
    }
    knockbackTimer_ -= _deltaTime;

    if (knockbackTimer_ <= 0.f) {
        knockback_ = false;
        knockbackTimer_ = 0.f;
    }
}

void Enemy::UpdateInvincible(float _deltaTime) {
    if (!invincible_ || !params_) return;

    invincibleTimer_ -= _deltaTime;

    // 進行度を計算 (1.0 → 0.0)
    float progress = invincibleTimer_ / params_->invincibleDuration;

    // HIT_FLASH_COLOR → BASE_COLOR にグラデーション
    float r = MathUtils::Lerp(GRAD_R_BASE, GRAD_FLASH, progress);
    float g = MathUtils::Lerp(GRAD_G_BASE, GRAD_FLASH, progress);
    float b = MathUtils::Lerp(GRAD_B_BASE, GRAD_FLASH, progress);
    model_->SetColor({r, g, b, 1.f});

    if (invincibleTimer_ <= 0.f) {
        invincible_ = false;
        invincibleTimer_ = params_->invincibleDuration;
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
