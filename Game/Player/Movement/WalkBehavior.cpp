#include "WalkBehavior.hpp"
#include "Input.hpp"

Vector3 WalkBehavior::Calculate(const Input* input, float deltaTime) {
    if (!input) return {};
    (void)deltaTime;

    Vector3 inputVector = GetInputVector(input);
    return inputVector * speed_;
}

bool WalkBehavior::CanExecute(const Input* input) {
    if (!input) return false;

    // WASD入力があれば実行可能
    return input->IsPress(DIK_W) ||
           input->IsPress(DIK_S) ||
           input->IsPress(DIK_A) ||
           input->IsPress(DIK_D);
}

Vector3 WalkBehavior::GetInputVector(const Input* input) const {
    if (!input) return {};

    Vector3 inputVector = {};

    // WASD入力
    if (input->IsPress(DIK_W)) inputVector.z += 1.0f;
    if (input->IsPress(DIK_S)) inputVector.z -= 1.0f;
    if (input->IsPress(DIK_A)) inputVector.x -= 1.0f;
    if (input->IsPress(DIK_D)) inputVector.x += 1.0f;

    // 正規化
    if (inputVector.Length() > 0.0f) {
        inputVector = inputVector.Normalize();
    }

    return inputVector;
}
