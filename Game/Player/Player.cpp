#include "Player.hpp"
#include <algorithm>
#include <optional>
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"
#include "CollisionAttribute.hpp"
#include "imgui.h"
#include "Json/JsonParams.hpp"
#include "Time/Time.hpp"

namespace {
    /** @brief マウスがこれより近いときはスキルを出さない */
    constexpr float MIN_AIM_DISTANCE = 0.01f;

    /** @brief プレイヤーを照らす点光源を、プレイヤーからどれだけ上に置くか */
    constexpr float LIGHT_HEIGHT = 3.f;
}

Player::Player(ParticleSystem* _particle, PostProcessExecutor* _postEffect, const KeyBinds& _binds) {
    particle_ = _particle;
    postEffect_ = _postEffect;

    position_ = { 0.f, .5f,-5.f };

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _collider) {OnCollision(_collider); })
        ->SetTranslate(position_)
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

    inputHandler_ = std::make_unique<InputHandler>();
    inputHandler_->Initialize(_binds);

    movement_ = std::make_unique<Movement>();
    movement_->Initialize(this);

    auto dash  = std::make_unique<DashBehavior>();
    auto walk  = std::make_unique<WalkBehavior>();
    dash_  = dash.get();
    walk_  = walk.get();
    behaviors_.push_back(std::move(dash));
    behaviors_.push_back(std::move(walk));

    for (auto& behavior : behaviors_) {
        movement_->AddBehavior(behavior.get());
    }

    reticle_ = std::make_unique<Sprite>();
    reticle_->Initialize("crosshair.png");
    reticle_->SetSize({32.f, 32.f});
    reticle_->SetColor({0.f, 0.7f, 1.f, 0.9f});

    UpdateLightAnchor();
}

void Player::Initialize() {
    // 値は Stage が SetStatus で入れる
    status_ = {};

    LoadParams();

    // ラウンドごとに状態を戻す
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
            RequestAutoAttack();

        RequestSkills();
    }

    UpdateInvulnerability(Time().GetDeltaTime());

    ApplyVelocity();
    ClampToFieldBounds();
    UpdateLightAnchor();

    collider_->SetTranslate(position_);

    UpdateModel();
}

void Player::Draw() {
    if (!model_) return;

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
}

void Player::SetOnSkillRequest(std::function<void(SkillSlot, const Vector3&, const Vector3&)> _callback) {
    onSkillRequest_ = std::move(_callback);
}

void Player::RequestSkills() {
    if (!inputHandler_ || !onSkillRequest_) return;

    std::optional<Vector3> direction;
    for (size_t i = 0; i < static_cast<size_t>(SkillSlot::Count); ++i) {
        const auto slot = static_cast<SkillSlot>(i);
        if (!inputHandler_->IsSkillRequested(slot)) continue;

        if (!direction) {
            Vector3 toMouse = inputHandler_->GetMouseInWorld() - position_;
            toMouse.y = 0.f;
            if (toMouse.Length() <= MIN_AIM_DISTANCE) return;
            direction = toMouse.Normalize();
        }
        onSkillRequest_(slot, position_, *direction);
    }
}

void Player::UpdateWithoutInput() {
    UpdateLightAnchor();

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
        json->TryGetValue("PlayerParams", "Walk", "Speed", baseWalkSpeed_);
        walk_->SetSpeed(baseWalkSpeed_);
    }
    if (dash_) {
        float speed = dash_->GetSpeed();
        float duration = dash_->GetDuration();
        float cooldown = dash_->GetCooldown();
        json->TryGetValue("PlayerParams", "Dash", "Speed", speed);
        json->TryGetValue("PlayerParams", "Dash", "Duration", duration);
        json->TryGetValue("PlayerParams", "Dash", "Cooldown", cooldown);
        dash_->SetSpeed(speed);
        dash_->SetDuration(duration);
        dash_->SetCooldown(cooldown);
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

    // 無敵が切れるにつれて、被弾の白から通常の色へ戻す
    const float remaining = invulnerabilityTimer_ / INVULNERABILITY_DURATION;
    model_->SetColor(MathUtils::Lerp(BASE_COLOR, DAMAGE_FLASH_COLOR, remaining));

    if (invulnerabilityTimer_ <= 0.f) {
        invulnerability_ = false;
        invulnerabilityTimer_ = INVULNERABILITY_DURATION;
        model_->SetColor(BASE_COLOR);
    }
}

void Player::UpdateLightAnchor() {
    forLight_ = position_;
    forLight_.y += LIGHT_HEIGHT;
}

void Player::RequestAutoAttack() {
    if (!targetExist_ || !onSkillRequest_) return;
    targetExist_ = false;

    Vector3 toTarget = targetPosition_ - position_;
    toTarget.y = 0.f;
    if (toTarget.Length() <= MIN_AIM_DISTANCE) return;

    onSkillRequest_(SkillSlot::AutoAttack, position_, toTarget.Normalize());
}
