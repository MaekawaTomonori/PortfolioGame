#include "Attack.hpp"

#include <algorithm>
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"
#include "Time/Time.hpp"

#undef min
#undef max

void Attack::Initialize() {
    timer_ = 0.f;
    bullets_.clear();
}

void Attack::Update() {
    std::erase_if(bullets_, [](const auto& bullet) { return !bullet->IsActive(); });
    for (auto& bullet : bullets_) {
        bullet->Update();
    }

    // 狙う相手がいない間も進めておき、現れたらすぐ撃てるよう 0 で止める
    timer_ = std::max(0.f, timer_ - Time().GetDeltaTime());
}

void Attack::Fire() {
    if (timer_ > 0.f) return;

    timer_ = attackRate_;
    Execute();
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

void Attack::Execute() {
    if (!owner_ || !owner_->IsActive()) return;

    direction_.y = 0.f;

    if (status_) {
        bulletParams_.damage = status_->get().damage;
    }

    auto atk = std::make_unique<Bullet>();
    atk->Initialize(owner_, direction_, bulletParams_);
    bullets_.emplace_back(std::move(atk));
}

void Attack::LoadParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("PlayerParams")) return;

    baseAttackRate_       = std::get<float>(json->GetValue("PlayerParams", "Attack",  "Rate"));
    attackRate_           = baseAttackRate_;
    minAttackInterval_    = std::get<float>(json->GetValue("PlayerParams", "Attack",  "MinInterval"));
    bulletParams_.speed   = std::get<float>(json->GetValue("PlayerParams", "Bullet",  "Speed"));
    bulletParams_.lifetime= std::get<float>(json->GetValue("PlayerParams", "Bullet",  "Lifetime"));
}

void Attack::ApplyAttackSpeed(const float _as) {
    if (_as > 0.f) {
        attackRate_ = std::max(minAttackInterval_, baseAttackRate_ / _as);
    }
}

void Attack::SaveParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->SetValue("PlayerParams", "Attack", "Rate",             baseAttackRate_);
    json->SetValue("PlayerParams", "Attack", "MinInterval",      minAttackInterval_);
    json->SetValue("PlayerParams", "Bullet", "Speed",            bulletParams_.speed);
    json->SetValue("PlayerParams", "Bullet", "Lifetime",         bulletParams_.lifetime);
    json->Save("PlayerParams");
}
