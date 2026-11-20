#include "Bullet.hpp"

#include "Time/Time.hpp"

namespace {
    constexpr float BULLET_SIZE = 0.3f;
}

Bullet::Bullet(const Params& _params, const Vector3& _position, const Vector3& _direction)
    : params_(_params) {
    position_ = _position;
    velocity_ = _direction.Normalize() * params_.speed;
    lifetime_ = params_.lifetime;
    damageSource_.SetDamage(params_.damage);

    scale_ = { BULLET_SIZE, BULLET_SIZE, BULLET_SIZE };
    SetModel("animatedCube");
    model_->SetTexture("white_x16.png");
    model_->SetName("Bullet");
    UpdateModel();

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _pCol) { this->OnCollision(_pCol); })
        ->SetType(Collision::Type::Sphere)
        ->SetSize(BULLET_SIZE)
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->AddAttribute(static_cast<uint32_t>(attribute_))
        ->AddIgnore(static_cast<uint32_t>(ignoreAttribute_))
        ->AddIgnore(static_cast<uint32_t>(attribute_))
        ->Enable();

    damageSource_.AttachTo(*collider_);
}

void Bullet::Update() {
    const float dt = Time().GetDeltaTime();

    position_ += velocity_ * dt;
    lifetime_ -= dt;
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

void Bullet::OnCollision(const Collision::Collider* _collider) {
    if (_collider->GetAttribute() & static_cast<uint32_t>(targetAttribute_)){
        if (piercing_ > 0) {
            piercing_--;
        } else {
            active_ = false;
            collider_->Disable();
        }
    }
}
