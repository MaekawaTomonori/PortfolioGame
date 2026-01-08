#include "Player.hpp"
#include "ColliderType.hpp"
#include "imgui_internal.h"
#include "Pattern/Singleton.hpp"
#include "Light/LightManager.hpp"
#include "Movement/WalkBehavior.hpp"
#include "Movement/DashBehavior.hpp"
#include "Movement/FlashBehavior.hpp"
#include "Math/MathUtils.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"

Player::Player(ParticleSystem* _particle, PostProcessExecutor* _postEffect) {
    particle_ = _particle;
    postEffect_ = _postEffect;

    inputHandler_ = std::make_unique<InputHandler>();
    inputHandler_->Initialize();

    movement_ = std::make_unique<Movement>();
    movement_->Initialize(this);

    behaviors_.push_back(std::make_unique<FlashBehavior>(5.0f, 3.0f));
    behaviors_.push_back(std::make_unique<DashBehavior>(10.0f, 0.3f, 1.0f));
    behaviors_.push_back(std::make_unique<WalkBehavior>(5.0f));

    for (auto& behavior : behaviors_) {
        movement_->AddBehavior(behavior.get());
    }

    particle_->Register("hit", { 3.f, 2.f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.5f,
            .duration = 0.7f,
            .spawnCount = 15,
            .size = {0.3f, 0.3f, 0.3f},
            .velocity = {0.f, 0.f, 0.f},
            .color = { 0.f, 0.2f, 1.f, 0.9f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // ランダムな方向に爆発（初回のみ設定）
                if (t < 0.01f) {
                    velocity = Vector3::Random() * 8.0f;
                }
                // 減速と重力
                velocity = velocity * 0.92f;
                velocity.y -= 0.05f;
                // フェードアウト
                color.w = 0.9f * (.7f - t);
            }
        });
    
    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _collider) {OnCollision(_collider); })
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->SetType(Collision::Type::AABB)
        ->SetSize(Collision::Vec3{ 1.f, 1.f, 1.f })
        ->SetOwner(this)
        ->AddAttribute(static_cast<uint32_t>(CollisionType::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::P_Bullet))
        ->Enable();

    SetModel("animatedcube");
    model_->SetTexture("white_x16.png");
    model_->SetColor(BaseColor);

    attack_ = std::make_unique<Attack>();
}

void Player::Initialize() {
    status_ = {
        10.f,
        1.f,
        1.f,
        1.f
    };

    position_ = {0.f, 1.5f,-5.f};

    attack_->Initialize();
    attack_->SetOwner(this);

    Singleton<LightManager>::GetInstance()->SetPosition(forlight_);
}

void Player::Update(float deltaTime) {
    if (!active_) return;

    if (inputHandler_) {
        inputHandler_->UpdateContext(movementContext_, position_);
    }

#ifdef _DEBUG
    if (!no_move)
#endif
        if (movement_) {
            movement_->Update(movementContext_, deltaTime);
        }

#ifdef _DEBUG
    if (!no_atk)
#endif
        UpdateAttack();

    UpdateInvulnerability(deltaTime);

    ApplyVelocity(deltaTime);

    forlight_ = position_;
    forlight_.y += 3.0f;
    Singleton<LightManager>::GetInstance()->SetPosition(forlight_);

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
    ImGui::End();
#endif
}

void Player::SetTargetPosition(Vector3 _position) {
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
            particle_->Edit("hit").SetPosition(p).Emit();

            model_->SetColor(DamageFlashColor);

            invulnerability_ = true;
            invulnerabilityTimer_ = InvulnerabilityDuration;
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

void Player::UpdateInvulnerability(float deltaTime) {
    if (!invulnerability_) return;

    invulnerabilityTimer_ -= deltaTime;

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
