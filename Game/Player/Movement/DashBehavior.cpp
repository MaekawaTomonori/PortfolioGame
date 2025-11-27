#include "DashBehavior.hpp"
#include "Input.hpp"

Vector3 DashBehavior::Calculate(const Input* _input, const float _deltaTime) {
    if (!_input) return {};

    // クールダウン更新
    if (currentCooldown_ > 0.0f) {
        currentCooldown_ -= _deltaTime;
    }

    // ダッシュ開始
    if (!isDashing_ && CanExecute(_input)) {
        isDashing_ = true;
        currentDuration_ = duration_;
        dashDirection_ = GetInputVector(_input);
    }

    // ダッシュ中
    if (isDashing_) {
        currentDuration_ -= _deltaTime;

        // ダッシュ終了
        if (currentDuration_ <= 0.0f) {
            isDashing_ = false;
            currentCooldown_ = cooldown_;
            return {};
        }

        return dashDirection_ * speed_;
    }

    return {};
}

bool DashBehavior::CanExecute(const Input* _input) const {
    if (!_input) return false;

    // クールダウン中は実行不可
    if (currentCooldown_ > 0.0f) return false;

    // ダッシュ中は実行不可
    if (isDashing_) return true;

    // Shift + 移動キー入力でダッシュ可能
    bool hasShift = _input->IsPress(DIK_LSHIFT);
    bool hasMovement = _input->IsPress(DIK_W) ||
                       _input->IsPress(DIK_S) ||
                       _input->IsPress(DIK_A) ||
                       _input->IsPress(DIK_D);

    return hasShift && hasMovement;
}

Vector3 DashBehavior::GetInputVector(const Input* _input) const {
    if (!_input) return {};

    Vector3 inputVector = {};

    // WASD入力
    if (_input->IsPress(DIK_W)) inputVector.z += 1.0f;
    if (_input->IsPress(DIK_S)) inputVector.z -= 1.0f;
    if (_input->IsPress(DIK_A)) inputVector.x -= 1.0f;
    if (_input->IsPress(DIK_D)) inputVector.x += 1.0f;

    // 正規化
    if (inputVector.Length() > 0.0f) {
        inputVector = inputVector.Normalize();
    }

    return inputVector;
}
