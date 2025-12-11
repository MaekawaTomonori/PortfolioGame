#include "Enemy.hpp"

#include "ColliderType.hpp"
#include "imgui.h"

void Enemy::Initialize() {
    SetModel("animatedcube");
    model_->SetTexture("white_x16.png");
    model_->SetColor({1.f, 0.3f, 0.3f, 1.f});

    status_ = {
        .hp = 3.f,
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

    movement_ = std::make_unique<Movement>();
    movement_->Initialize(this);
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;

    // 死亡演出中は特別処理
    if (dying_) {
        UpdateDeath(deltaTime);
        UpdateModel();
        return;
    }

    // 各状態の更新
    UpdateMovement(deltaTime);
    UpdateInvincible(deltaTime);
    UpdateShake();
    UpdatePulse(deltaTime);

    ApplyVelocity(deltaTime);
    collider_->SetTranslate({position_.x, position_.y, position_.z});

    // 描画更新
    UpdateModel();
}

void Enemy::Draw() {
    if (!active_ || !model_) return;

    model_->Draw();

}

void Enemy::Debug() {
#ifdef _DEBUG
    if (ImGui::TreeNode("Enemy Info")) {
        // 基本情報
        ImGui::Text("Active: %s", active_ ? "Yes" : "No");
        ImGui::Text("Dying: %s", dying_ ? "Yes" : "No");

        ImGui::Separator();

        // ステータス
        if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("HP: %.1f", status_.hp);
            ImGui::ProgressBar(status_.hp / 3.0f, ImVec2(-1, 0), "");
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
                auto size = get<Collision::Vec3>(collider_->GetSize());
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
            if (invincible_) {
                ImGui::SameLine();
                ImGui::Text("(%.2fs)", invincibleTimer_);
                ImGui::ProgressBar(invincibleTimer_ / InvincibleDuration, ImVec2(-1, 0));
            }

            ImGui::Checkbox("Knockback", (bool*)&knockback_);
            if (knockback_) {
                ImGui::SameLine();
                ImGui::Text("(%.2fs)", knockbackTimer_);
                ImGui::ProgressBar(knockbackTimer_ / 0.15f, ImVec2(-1, 0));
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

        // 死亡演出
        if (dying_ && ImGui::CollapsingHeader("Death Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Timer: %.2fs / %.2fs", deathTimer_, DeathDuration);
            ImGui::ProgressBar(deathTimer_ / DeathDuration, ImVec2(-1, 0));

            float progress = deathTimer_ / DeathDuration;
            if (progress < 0.3f) {
                ImGui::Text("Phase: Expansion");
            } else {
                ImGui::Text("Phase: Shrink & Fade");
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

            ImGui::Text("Move Command: %s", moveCommand_ ? "Set" : "None");
            ImGui::Text("Movement: %s", movement_ ? "Active" : "Inactive");
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
                particle_->Edit("enemy_hit").SetPosition(p).Emit();
            }

            if (status_.hp <= 0.f) {
                // 死亡演出を開始
                dying_ = true;
                deathTimer_ = 0.f;
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

void Enemy::UpdateMovement(float _deltaTime) {
    if (knockback_) {
        UpdateKnockback(_deltaTime);
    } else {
        movementContext_.Reset();
        movementContext_.position = position_;
        movementContext_.target = target_;

        if (moveCommand_) {
            moveCommand_->Execute(movementContext_);
        }

        if (movement_) {
            movement_->Update(movementContext_, _deltaTime);
        }

        if (invincible_) {
            velocity_ *= 0.3f;
        }
    }
}

void Enemy::UpdateKnockback(float _deltaTime) {
    velocity_ *= KnockbackDecay;
    knockbackTimer_ -= _deltaTime;

    if (knockbackTimer_ <= 0.f) {
        knockback_ = false;
        knockbackTimer_ = 0.f;
    }
}

void Enemy::UpdateInvincible(float _deltaTime) {
    if (!invincible_) return;

    invincibleTimer_ -= _deltaTime;

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

void Enemy::UpdateDeath(float _deltaTime) {
    deathTimer_ += _deltaTime;

    if (deathTimer_ >= DeathDuration) {
        // 演出終了、敵を削除
        active_ = false;
        return;
    }

    float progress = deathTimer_ / DeathDuration;  // 0.0 → 1.0

    // フェーズ1 (0.0-0.3): 膨張しながら光る
    // フェーズ2 (0.3-0.8): 縮小しながらフェードアウト

    if (progress < 0.3f) {
        // === フェーズ1: 膨張 ===
        float phase1Progress = progress / 0.3f;  // 0.0 → 1.0

        // スケール: 1.0 → 1.3 に膨張
        float scaleValue = MathUtils::Lerp(1.0f, 1.3f, phase1Progress);
        scale_ = Vector3{scaleValue, scaleValue, scaleValue};

        // 色: 白く光る
        model_->SetColor({2.f, 2.f, 2.f, 1.f});

        // 回転: Y軸で加速回転
        rotation_.y += _deltaTime * 10.0f * (1.0f + phase1Progress * 2.0f);

    } else {
        // === フェーズ2: 縮小＆フェードアウト ===
        float phase2Progress = (progress - 0.3f) / 0.5f;  // 0.0 → 1.0

        // スケール: 1.5 → 0.0 に縮小
        float scaleValue = MathUtils::Lerp(1.5f, 0.0f, phase2Progress);
        scale_ = Vector3{scaleValue, scaleValue, scaleValue};

        // 色: 白 → 透明へフェードアウト
        float alpha = 1.0f - phase2Progress;
        model_->SetColor({2.f, 2.f, 2.f, alpha});

        // 回転: さらに加速
        rotation_.y += _deltaTime * 20.0f;

        // 位置: 少し浮き上がる
        position_.y += _deltaTime * 2.0f;
    }
}

