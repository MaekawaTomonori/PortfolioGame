#include "Enemy.hpp"

Enemy::Enemy()
    : behavior_(std::make_unique<BehaviorComponent>())
    , model_(nullptr) {
}

Enemy::~Enemy() {
}

void Enemy::Initialize() {
    behavior_->Initialize(this);

    model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
    model_->SetEnvironmentTexture("Skybox.dds");
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;
    
    // BehaviorComponentから移動量を取得
    Vector3 deltaMovement = behavior_->Update(deltaTime);
    
    // 位置を更新
    SetPosition(position_ + deltaMovement);

    if (model_) {
        model_->Update();
    }
}

void Enemy::Draw() {
    if (!active_ || !model_) return;
    
    model_->Draw();
}

void Enemy::SetTarget(GameObject* target) {
    behavior_->SetTarget(target);
}

GameObject* Enemy::GetTarget() const {
    return behavior_->GetTarget();
}