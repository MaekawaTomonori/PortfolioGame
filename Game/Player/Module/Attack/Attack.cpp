#include "Attack.hpp"

#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"

void Attack::Initialize() {
    timer_ = 0.f;
}

void Attack::Update() {
    std::erase_if(bullets_, [](const auto& bullet) { return !bullet->IsActive(); });
    for (auto& bullet : bullets_) {
        bullet->Update(DELTA_TIME);
    }

    if (timer_ < 0.f) {
        timer_ = attackRate_;

        // Attack
        Execute();

        return;
    }

    timer_ -= DELTA_TIME;
}

void Attack::Draw() {
    for (const auto& bullet : bullets_) {
        bullet->Draw();
    }
}

void Attack::SetOwner(GameObject* _owner) {
    owner_ = _owner;
}

void Attack::SetDirection(const Vector3& _direction) {
    direction_ = _direction;
}

void Attack::SetStatus(PlayerStatus& _status) {
    status_ = std::ref(_status);
}

void Attack::Clear() {
    bullets_.clear();
}

void Attack::Execute() {
    if (!owner_ && !owner_->IsActive()) return;

    direction_.y = 0.f;

    auto atk = std::make_unique<Bullet>();
    atk->Initialize(owner_, direction_, bulletParams_);
    bullets_.emplace_back(std::move(atk));
}

void Attack::LoadParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("PlayerParams")) return;

    attackRate_           = std::get<float>(json->GetValue("PlayerParams", "Attack",  "Rate"));
    bulletParams_.speed   = std::get<float>(json->GetValue("PlayerParams", "Bullet",  "Speed"));
    bulletParams_.lifetime= std::get<float>(json->GetValue("PlayerParams", "Bullet",  "Lifetime"));
}

void Attack::SaveParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->SetValue("PlayerParams", "Attack", "Rate",             attackRate_);
    json->SetValue("PlayerParams", "Bullet", "Speed",            bulletParams_.speed);
    json->SetValue("PlayerParams", "Bullet", "Lifetime",         bulletParams_.lifetime);
    json->Save("PlayerParams");
}
