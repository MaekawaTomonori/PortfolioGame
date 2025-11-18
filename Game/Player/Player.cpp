#include "Player.hpp"

#include <filesystem>

#include "ColliderType.hpp"
#include "Pattern/Singleton.hpp"
#include "Light/LightManager.hpp"

void Player::Initialize() {
    status_ = {
        10.f,
        1.f,
        1.f,
        1.f
    };

    // MovementComponentの初期化
    //movement_ = std::make_unique<MovementComponent>();
    //movement_->Initialize(this);

    // Attackモジュールの初期化
    //attack_ = std::make_unique<Attack>();
    //attack_->Initialize();
    //attack_->SetOwner(this);

    // モデルの初期化
    SetModel("animatedcube");
    model_->SetTexture("white_x16.png");
    model_->SetColor({ 0.3f, 0.3f, 1.f, 1.f });

    // Collision
    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _collider) {OnCollision(_collider); })
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->SetType(Collision::Type::AABB)
        ->SetSize(Collision::Vec3{ 1.f, 1.f, 1.f })
        ->SetOwner(this)
        ->AddAttribute(static_cast<uint32_t>(CollisionType::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::P_Bullet))
        ->Enable();

    Singleton<LightManager>::GetInstance()->SetReference(forlight_);
}

void Player::Update(float deltaTime) {
    if (!active_) return;

    // MovementComponentでvelocityを設定
    if (movement_) {
        movement_->Update();
    }

    //Attackモジュールの更新
    if (attack_) {
        attack_->SetDirection((targetPosition_ - position_).Normalize());
        attack_->Update();
    }

    if (invulnerability_) {
        invulnerabilityTimer_ -= deltaTime;
        if (invulnerabilityTimer_ <= 0.f) {
            invulnerability_ = false;
            invulnerabilityTimer_ = 1.f;
        }
    }

    // velocityを位置に適用
    ApplyVelocity(deltaTime);

    forlight_ = position_;
    forlight_.y += 3.0f;

    collider_->SetTranslate({ position_.x, position_.y, position_.z });

    UpdateModel();
}

void Player::Draw() {
    if (!active_ || !model_) return;

    if (attack_) {
        attack_->Draw();
    }

    model_->Draw();
}

void Player::SetTargetPosition(Vector3 _position) {
    targetPosition_ = _position;
}

void Player::OnCollision(const Collision::Collider* _collider) {
    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::Enemy)) {
        if (invulnerability_)return;
        if (0.f < status_.hp) {
            status_.hp -= 1.f;
            invulnerability_ = true;
        }
        else {
            active_ = false;
        }
    }
}
