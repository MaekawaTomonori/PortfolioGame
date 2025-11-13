#include "Enemies.hpp"

#include "Command/Move/ToTargetCommand.hpp"

void Enemies::Initialize() {
    timer_ = 0.f;
}

void Enemies::Update() {
    constexpr float DeltaTime = 1.f / 60.f;

    std::erase_if(enemies_, [](const auto& _enemy) {return !_enemy->IsActive();});

    if (Interval <= timer_) {
        timer_ = 0.f;
        Spawn();
    } else {
        timer_ += DeltaTime;
    }

    for (const auto& enemy : enemies_) {
        enemy->Update(DeltaTime);
    }
}

void Enemies::Draw() const {
    for (const auto& enemy : enemies_) {
        enemy->Draw();
    }
}

Vector3 Enemies::GetNearest(const Vector3 _pos) const {
    float minDistance = FLT_MAX;
    Vector3 nearest = {};
    for (const auto& enemy : enemies_) {
        Vector3 toEnemy = enemy->GetPosition() - _pos;
        if (toEnemy.Length() < minDistance) {
            minDistance = toEnemy.Length();
            nearest = enemy->GetPosition();
        }
    }
    return nearest;
}

void Enemies::SetTarget(GameObject* _target) {
    target_ = _target;
}

void Enemies::Spawn() {
    if (MaxEnemies <= enemies_.size()) return;

    std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetTarget(target_);
    enemy->SetMoveCommand(std::make_unique<ToTargetCommand>());
    enemies_.push_back(std::move(enemy));
}
