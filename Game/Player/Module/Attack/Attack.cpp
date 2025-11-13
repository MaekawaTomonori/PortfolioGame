#include "Attack.hpp"

void Attack::Initialize() {
    timer_ = 0.f;
}

void Attack::Update() {
    std::erase_if(bullets_, [](const auto& bullet) { return !bullet->IsActive(); });
    for (auto& bullet : bullets_) {
        bullet->Update(DeltaTime);
    }

    if (timer_ < 0.f) {
        timer_ = BaseAttackRate;

        // Attack
        Execute();

        return;
    }

    timer_ -= DeltaTime;
}

void Attack::Draw() {
    for (const auto& bullet : bullets_) {
        bullet->Draw();
    }
}

void Attack::SetOwner(GameObject* _owner) {
    owner_ = _owner;
}

void Attack::SetDirection(const Vector3 _direction) {
    direction_ = _direction;
}

void Attack::SetStatus(PlayerStatus& _status) {
    status_ = std::ref(_status);
}

void Attack::Execute() {
    if (!owner_ && !owner_->IsActive()) return;

    direction_.y = 0.f;

    auto atk = std::make_unique<Bullet>();
    atk->Initialize(owner_, direction_);
    bullets_.emplace_back(std::move(atk));
}
