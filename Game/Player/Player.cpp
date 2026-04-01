#include "Player.hpp"
#include "Pattern/Singleton.hpp"
#include "Light/LightManager.hpp"
#include "Math/MathUtils.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"
#include "ColliderType.hpp"
#include "Json/JsonParams.hpp"

#include "imgui_internal.h"

Player::Player(ParticleSystem* _particle, PostProcessExecutor* _postEffect) {
    particle_ = _particle;
    postEffect_ = _postEffect;

    position_ = { 0.f, .5f,-5.f };

    particle_->RegisterUpdateFunc("hit_explosion", [](float t, const Vector3&, Vector3& pos, Vector3& velocity, Vector4& color) {
        (void)pos;
        if (t < 0.01f) {
            velocity = Vector3::Random() * 8.0f;
        }
        velocity = velocity * 0.92f;
        velocity.y -= 0.05f;
        color.w = 0.9f * (.7f - t);
    });

    particle_->Register("hit");
    
    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _collider) {OnCollision(_collider); })
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->SetType(Collision::Type::AABB)
        ->SetSize(Vector3{ .3f, .3f, .3f })
        ->SetOwner(this)
        ->AddAttribute(static_cast<uint32_t>(CollisionType::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::P_Bullet))
        ->Enable();

    SetModel("animatedCube");
    model_->SetTexture("white_x16.png");
    model_->SetColor(BaseColor);
    model_->SetName("Player");
    SetScale({0.4f, 0.4f, 0.4f});
    UpdateModel();

    // Modules
    inputHandler_ = std::make_unique<InputHandler>();
    movement_ = std::make_unique<Movement>();
    attack_ = std::make_unique<Attack>();

    reticle_ = std::make_unique<Model>();
    reticle_->Initialize("animatedCube");
    reticle_->SetTexture("white_x16.png");
    reticle_->SetColor({1.f, 0.f, 0.f, 1.f});
    reticle_->SetScale({0.3f, 0.3f, 0.3f});
}

void Player::Initialize() {
    status_ = {
        100.f,
        1.f,
        1.f,
        1.f
    };

    inputHandler_->Initialize();

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

    attack_->Initialize();
    attack_->SetOwner(this);

    LoadParams();
    attack_->LoadParams();

    forLight_ = position_;
    forLight_.y += 3.f;
    Singleton<LightManager>::GetInstance()->SetPosition(forLight_);
}

void Player::Update(const float _deltaTime) {
    if (!active_) return;

    if (inputHandler_) {
        inputHandler_->UpdateContext(movementContext_, position_);
    }

    if (reticle_) {
        reticle_->SetTranslate(movementContext_.targetPosition);
    }
#ifdef _DEBUG

    if (!no_move)
#endif
        if (movement_) {
            movement_->Update(movementContext_, _deltaTime);
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

    UpdateInvulnerability(_deltaTime);

    ApplyVelocity(_deltaTime);

    forLight_ = position_;
    forLight_.y += 3.0f;
    Singleton<LightManager>::GetInstance()->SetPosition(forLight_);

    collider_->SetTranslate({ position_.x, position_.y, position_.z });

    UpdateModel();
}

void Player::Draw() {
    if (!active_ || !model_) return;

    if (attack_) {
#ifdef _DEBUG
        if (!no_atk) 
#endif
          attack_->Draw();
    }


    if (reticle_) {
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
    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::Enemy)) {
        if (invulnerability_)return;
        if (0.f < status_.hp) {
            //Damage Motion
            if (camera_) camera_->Shake(0.4f, 1.f);

            if (postEffect_) postEffect_->ApplyPreset("DamageScreen");

            Vector3 p = position_;
            p.y += 1.5f;
            particle_->Emit("hit", p);

            model_->SetColor(DamageFlashColor);

            invulnerability_ = true;
            invulnerabilityTimer_ = InvulnerabilityDuration;

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
}

void Player::UpdateWithoutInput() {
    forLight_ = position_;
    forLight_.y += 3.0f;
    Singleton<LightManager>::GetInstance()->SetPosition(forLight_);

    if (model_) {
        UpdateModel();
    }
}

bool Player::IsDead() const {
    return status_.hp <= 0.f;
}

void Player::LoadParams() const {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("PlayerParams")) return;

    if (walk_)  walk_->SetSpeed   (std::get<float>(json->GetValue("PlayerParams", "Walk",  "Speed")));
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

void Player::UpdateInvulnerability(const float _deltaTime) {
    if (!invulnerability_) return;

    invulnerabilityTimer_ -= _deltaTime;

    // 進行度を計算 (1.0 → 0.0)
    float progress = invulnerabilityTimer_ / InvulnerabilityDuration;

    // 白(2.0, 2.0, 2.0) → 青(0.3, 0.3, 1.0) にグラデーション
    Vector4 currentColor;
    currentColor.x = MathUtils::Lerp(BaseColor.x, DamageFlashColor.x, progress);
    currentColor.y = MathUtils::Lerp(BaseColor.y, DamageFlashColor.y, progress);
    currentColor.z = MathUtils::Lerp(BaseColor.z, DamageFlashColor.z, progress);
    currentColor.w = 1.f;
    model_->SetColor(currentColor);

    if (invulnerabilityTimer_ <= 0.f) {
        invulnerability_ = false;
        invulnerabilityTimer_ = InvulnerabilityDuration;
        model_->SetColor(BaseColor);
    }
}

void Player::UpdateAttack() {
    if (!attack_) return; 
    if (!targetExist_) {
        attack_->Clear();
        return;
    }

    attack_->SetDirection((targetPosition_ - position_).Normalize());
    attack_->Update();
    targetExist_ = false;
}
