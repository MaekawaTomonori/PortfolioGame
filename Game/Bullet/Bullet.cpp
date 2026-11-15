#include "Bullet.hpp"

#include "CollisionAttribute.hpp"
#include "Time/Time.hpp"

void Bullet::Initialize(GameObject* _owner, const Vector3& _direction, const Params& _params,
    CollisionAttribute _attribute, CollisionAttribute _ignoreAttribute, CollisionAttribute _targetAttribute) {
    speed_    = _params.speed;
    lifetime_ = _params.lifetime;
    damageSource_.SetDamage(_params.damage);
    owner_    = _owner;
    velocity_  = _direction.Normalize() * speed_;
    attribute_ = _attribute;
    ignoreAttribute_ = _ignoreAttribute;
    targetAttribute_ = _targetAttribute;
    Initialize();
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
        ->SetType(Collision::Type::Sphere)
        ->SetSize(0.3f)
        ->SetTranslate({ position_.x, position_.y, position_.z })
        ->AddAttribute(static_cast<uint32_t>(attribute_))
        ->AddIgnore(static_cast<uint32_t>(ignoreAttribute_))
        ->AddIgnore(static_cast<uint32_t>(attribute_))
        ->Enable();

    damageSource_.AttachTo(*collider_);
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
