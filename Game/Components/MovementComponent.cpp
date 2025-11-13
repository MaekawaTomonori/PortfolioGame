#include "MovementComponent.hpp"
#include "GameObject/GameObject.hpp"
#include <dinput.h>

#include "Pattern/Singleton.hpp"

MovementComponent::MovementComponent() : input_(nullptr), owner_(nullptr) {
}

MovementComponent::~MovementComponent() {
}

void MovementComponent::Initialize(GameObject* owner) {
    owner_ = owner;
    input_ = Singleton<Input>::GetInstance();
}

void MovementComponent::Update() {
    if (!owner_) return;

    Vector3 inputVector = GetInputVector();
    Vector3 velocity = inputVector * moveSpeed_;

    // GameObjectのvelocityに設定
    owner_->SetVelocity(velocity);
}

Vector3 MovementComponent::GetInputVector() const {
    if (!input_) return {0.0f, 0.0f, 0.0f};
    
    Vector3 inputVector{0.0f, 0.0f, 0.0f};
    
    // WASD入力
    if (input_->IsPress(DIK_W)) inputVector.z += 1.0f;
    if (input_->IsPress(DIK_S)) inputVector.z -= 1.0f;
    if (input_->IsPress(DIK_A)) inputVector.x -= 1.0f;
    if (input_->IsPress(DIK_D)) inputVector.x += 1.0f;
    
    if (inputVector.Length() > 0.0f) {
        inputVector = inputVector.Normalize();
    }
    
    return inputVector;
}