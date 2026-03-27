#include "Bullet.hpp"

#include "ColliderType.hpp"

void Bullet::Initialize(GameObject* _owner, const Vector3& _direction, const Params& _params) {
    speed_    = _params.speed;
    lifetime_ = _params.lifetime;
    owner_    = _owner;
    velocity_  = _direction.Normalize() * speed_;
    Initialize();
}

void Bullet::Update(float deltaTime) {
    position_ += velocity_ * deltaTime;
    lifetime_ -= deltaTime;
    if (lifetime_ <= 0.0f) {
        active_ = false;
    }

    collider_->SetTranslate({position_.x, position_.y, position_.z} );
    UpdateModel();
}

void Bullet::Draw() {
    if (!active_ || !model_)return;

    model_->Draw();
}

void Bullet::Initialize() {
    type_ = owner_->GetType();
    position_ = owner_->GetPosition();
    scale_ = { 0.3f, 0.3f, 0.3f };
    SetModel("animatedCube");
    model_->SetTexture("white_x16.png");
    model_->SetName("Bullet");
    UpdateModel();

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _pCol) { this->OnCollision(_pCol); })
        ->SetOwner(this)
        ->SetType(Collision::Type::Sphere)
        ->SetSize(0.3f)
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->AddAttribute(static_cast<uint32_t>(CollisionType::P_Bullet))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::P_Bullet))
        ->Enable();
}

void Bullet::OnCollision(const Collision::Collider* _collider) {
    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::Enemy)){
        if (piercing_ > 0) {
            piercing_--;
        } else {
            active_ = false;
            collider_->Disable();
        }
    }
}
