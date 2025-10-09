#include "MovementComponent.hpp"
#include <dinput.h>

MovementComponent::MovementComponent() : input_(nullptr) {
}

MovementComponent::~MovementComponent() {
}

void MovementComponent::Initialize() {
    input_ = Singleton<Input>::GetInstance();
}

Vector3 MovementComponent::Update(float deltaTime) {
    Vector3 inputVector = GetInputVector();
    velocity_ = inputVector * moveSpeed_;
    
    return velocity_ * deltaTime;
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