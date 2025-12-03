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
    if (moveCommand_ && target_) {
        moveCommand_->Execute(this, target_);
    }

    if (invincible_) {
        invincibleTimer_ -= deltaTime;

        velocity_ *= 0.7f;

        if (invincibleTimer_ <= 0.f) {
            invincible_ = false;
            invincibleTimer_ = 0.3f;
        }
    }

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
        --status_.hp;
        invincible_ = true;

        if (status_.hp <= 0.f) {
            active_ = false;
            collider_->Disable();
        }
    }
}
