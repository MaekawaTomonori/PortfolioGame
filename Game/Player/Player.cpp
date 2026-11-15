#include "Player.hpp"
#include <algorithm>
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"
#include "CollisionAttribute.hpp"
#include "imgui.h"
#include "Json/JsonParams.hpp"
#include "Time/Time.hpp"

#undef min
#undef max

Player::Player(ParticleSystem* _particle, PostProcessExecutor* _postEffect) {
    particle_ = _particle;
    postEffect_ = _postEffect;

    position_ = { 0.f, .5f,-5.f };

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _collider) {OnCollision(_collider); })
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->SetType(Collision::Type::AABB)
        ->SetSize(Vector3{ .3f, .3f, .3f })
        ->SetOwner(this)
        ->AddAttribute(static_cast<uint32_t>(CollisionAttribute::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionAttribute::PlayerBullet))
        ->Enable();

    SetModel("animatedCube");
    model_->SetTexture("white_x16.png");
    model_->SetColor(BASE_COLOR);
    model_->SetName("Player");
    SetScale({0.4f, 0.4f, 0.4f});
    UpdateModel();

    // Modules Initialization

    // InputModule Initialize
    inputHandler_ = std::make_unique<InputHandler>();
    inputHandler_->Initialize();

    // MovementModule Initialize
    movement_ = std::make_unique<Movement>();
    movement_->Initialize(this);

    auto flash = std::make_unique<FlashBehavior>();
    auto dash  = std::make_unique<DashBehavior>();
    auto walk  = std::make_unique<WalkBehavior>();
    flash_ = flash.get();
    dash_  = dash.get();
    walk_  = walk.get();
    behaviors_.push_back(std::move(flash));
    behaviors_.push_back(std::move(dash));
    behaviors_.push_back(std::move(walk));

    for (auto& behavior : behaviors_) {
        movement_->AddBehavior(behavior.get());
    }

    // AttackModule Initialize
    attack_ = std::make_unique<Attack>();
    attack_->Initialize();
    attack_->SetOwner(this);
    attack_->SetStatus(status_);
    
    reticle_ = std::make_unique<Sprite>();
    reticle_->Initialize("crosshair.png");
    reticle_->SetSize({32.f, 32.f});
    reticle_->SetColor({0.f, 0.7f, 1.f, 0.9f});

    forLight_ = position_;
    forLight_.y += 3.f;
}

void Player::Initialize() {
    status_ = {
        100.f,
        1.f,
        1.f,
        1.f
    };

    LoadParams();
    attack_->LoadParams();
    // 前のラウンドの弾を持ち越さない
    attack_->Initialize();

    // 振る舞いは1回しか作らないので、クールダウンや被弾後の無敵をラウンドごとに戻す
    for (auto& behavior : behaviors_) {
        behavior->Reset();
    }
    invulnerability_ = false;
    invulnerabilityTimer_ = 0.f;
    model_->SetColor(BASE_COLOR);
}

void Player::Update() {
    if (!active_) return;

    if (enable_) {
        if (inputHandler_) {
            inputHandler_->UpdateContext(movementContext_, position_);
        }

        if (reticle_) {
            reticle_->SetPosition(Singleton<Input>::GetInstance()->GetMousePosition());
            reticle_->Update();
        }

#ifdef _DEBUG
        if (!no_move)
#endif
            if (movement_) {
                movement_->Update(movementContext_);
            }

#ifdef _DEBUG
        if (!no_atk)
#endif
            UpdateAttack();

        // スキル発動
        if (movementContext_.isSkillRequested && onSkillRequest_) {
            Vector3 dir = (movementContext_.targetPosition - position_);
            dir.y = 0.f;
            if (dir.Length() > 0.01f) {
                onSkillRequest_(position_, dir.Normalize());
            }
        }
    }

    UpdateInvulnerability(Time().GetDeltaTime());

    ApplyVelocity();
    ClampToFieldBounds();

    forLight_ = position_;
    forLight_.y += 3.0f;

    collider_->SetTranslate({ position_.x, position_.y, position_.z });

    UpdateModel();
}

void Player::Draw() {
    if (!model_) return;

    if (attack_) {
#ifdef _DEBUG
        if (!no_atk) 
#endif
          attack_->Draw();
    }

    if (active_ && enable_ && reticle_) {
        reticle_->Draw();
    }

    model_->Draw();
}

void Player::Debug() {
    if (movement_){
        movement_->Debug();
    }

#ifdef _DEBUG
    ImGui::Begin("Player");
    ImGui::Checkbox("No Move", &no_move);
    ImGui::Checkbox("No Attack", &no_atk);
    Vector3 scale = GetScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) {
        SetScale(scale);
    }

    ImGui::End();
#endif
}

void Player::SetTargetPosition(const Vector3& _position) {
    targetPosition_ = _position;
    targetExist_ = true;
}

void Player::OnCollision(const Collision::Collider* _collider) {
    if (_collider->GetAttribute() & (static_cast<uint32_t>(CollisionAttribute::Enemy) | static_cast<uint32_t>(CollisionAttribute::EnemyBullet))) {
        if (invulnerability_)return;
        if (0.f < status_.hp) {
            //Damage Motion
            if (camera_) camera_->Shake(0.4f, 1.f);

            if (postEffect_) postEffect_->ApplyPreset("DamageScreen");

            Vector3 p = position_;
            p.y += 1.5f;
            particle_->Emit("hit", p);

            model_->SetColor(DAMAGE_FLASH_COLOR);

            invulnerability_ = true;
            invulnerabilityTimer_ = INVULNERABILITY_DURATION;

            status_.hp -= 1.f;
        }
        else {
            active_ = false;
        }
    }
}

void Player::SetCamera(FollowCamera* _camera) {
    camera_ = _camera;
}

void Player::SetStatus(const PlayerStatus& _status) {
    status_ = _status;
    if (walk_) walk_->SetSpeed(baseWalkSpeed_ * _status.ms);
    if (attack_) attack_->ApplyAttackSpeed(_status.as);
}

void Player::UpdateWithoutInput() {
    forLight_ = position_;
    forLight_.y += 3.0f;

    if (model_) {
        UpdateModel();
    }
}

bool Player::IsDead() const {
    return status_.hp <= 0.f;
}

void Player::LoadParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("PlayerParams")) return;

    if (walk_) {
        baseWalkSpeed_ = std::get<float>(json->GetValue("PlayerParams", "Walk", "Speed"));
        walk_->SetSpeed(baseWalkSpeed_);
    }
    if (dash_) {
        dash_->SetSpeed   (std::get<float>(json->GetValue("PlayerParams", "Dash",  "Speed")));
        dash_->SetDuration(std::get<float>(json->GetValue("PlayerParams", "Dash",  "Duration")));
        dash_->SetCooldown(std::get<float>(json->GetValue("PlayerParams", "Dash",  "Cooldown")));
    }
    if (flash_) {
        flash_->SetDistance(std::get<float>(json->GetValue("PlayerParams", "Flash", "Distance")));
        flash_->SetCooldown(std::get<float>(json->GetValue("PlayerParams", "Flash", "Cooldown")));
    }
}

void Player::SaveParams() const {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (walk_)  json->SetValue("PlayerParams", "Walk",  "Speed",    walk_->GetSpeed());
    if (dash_) {
        json->SetValue("PlayerParams", "Dash",  "Speed",    dash_->GetSpeed());
        json->SetValue("PlayerParams", "Dash",  "Duration", dash_->GetDuration());
        json->SetValue("PlayerParams", "Dash",  "Cooldown", dash_->GetCooldown());
    }
    if (flash_) {
        json->SetValue("PlayerParams", "Flash", "Distance", flash_->GetDistance());
        json->SetValue("PlayerParams", "Flash", "Cooldown", flash_->GetCooldown());
    }
    attack_->SaveParams();
    json->Save("PlayerParams");
}

void Player::ClampToFieldBounds() {
    if (fieldHalfExtent_ <= 0.f) return;

    position_.x = std::clamp(position_.x, -fieldHalfExtent_, fieldHalfExtent_);
    position_.z = std::clamp(position_.z, -fieldHalfExtent_, fieldHalfExtent_);

    float distanceToEdge = fieldHalfExtent_ - std::max(std::abs(position_.x), std::abs(position_.z));
    bool nearEdge = distanceToEdge <= FIELD_EDGE_WARNING_DISTANCE;
    if (nearEdge == isNearFieldEdge_) return;
    isNearFieldEdge_ = nearEdge;

    if (!postEffect_) return;

    if (isNearFieldEdge_) {
        postEffect_->ApplyPreset("FieldEdge");
        return;
    }
    postEffect_->SetActive("FieldEdge", false);
}

void Player::UpdateInvulnerability(const float _deltaTime) {
    if (!invulnerability_) return;

    invulnerabilityTimer_ -= _deltaTime;

    // 進行度を計算 (1.0 → 0.0)
    float progress = invulnerabilityTimer_ / INVULNERABILITY_DURATION;

    // 白(2.0, 2.0, 2.0) → 青(0.3, 0.3, 1.0) にグラデーション
    Vector4 currentColor;
    currentColor.x = MathUtils::Lerp(BASE_COLOR.x, DAMAGE_FLASH_COLOR.x, progress);
    currentColor.y = MathUtils::Lerp(BASE_COLOR.y, DAMAGE_FLASH_COLOR.y, progress);
    currentColor.z = MathUtils::Lerp(BASE_COLOR.z, DAMAGE_FLASH_COLOR.z, progress);
    currentColor.w = 1.f;
    model_->SetColor(currentColor);

    if (invulnerabilityTimer_ <= 0.f) {
        invulnerability_ = false;
        invulnerabilityTimer_ = INVULNERABILITY_DURATION;
        model_->SetColor(BASE_COLOR);
    }
}

void Player::UpdateAttack() {
    if (!attack_) return;

    // 飛んでいる弾は、狙う相手がいなくなっても動かし続ける
    attack_->Update();
    if (!targetExist_) return;

    attack_->SetDirection((targetPosition_ - position_).Normalize());
    attack_->Fire();
    targetExist_ = false;
}
