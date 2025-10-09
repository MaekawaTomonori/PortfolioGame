#include "BehaviorComponent.hpp"
#include "GameObject/GameObject.hpp"

BehaviorComponent::BehaviorComponent()
    : owner_(nullptr), target_(nullptr) {
}

BehaviorComponent::~BehaviorComponent() {
}

void BehaviorComponent::Initialize(GameObject* owner) {
    owner_ = owner;
}

Vector3 BehaviorComponent::Update(float deltaTime) {
    // 基本的な動作：ターゲットに向かって移動
    if (!target_ || !owner_) {
        return Vector3{0.0f, 0.0f, 0.0f};
    }
    
    Vector3 currentPos = owner_->GetPosition();
    Vector3 targetPos = target_->GetPosition();
    Vector3 direction = targetPos - currentPos;
    
    float distance = direction.Length();
    if (distance > 0.1f) {
        direction = direction.Normalize();
        float moveSpeed = 2.0f;
        return direction * moveSpeed * deltaTime;
    }
    
    return Vector3{0.0f, 0.0f, 0.0f};
}