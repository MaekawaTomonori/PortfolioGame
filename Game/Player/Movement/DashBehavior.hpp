#ifndef DASH_BEHAVIOR_HPP_
#define DASH_BEHAVIOR_HPP_

#include "IMovementBehavior.hpp"

/// <summary>
/// ダッシュ動作
/// Shiftキーによる高速移動を実装
/// クールダウンと持続時間を持つ
/// </summary>
class DashBehavior : public IMovementBehavior {
    float speed_ = 10.0f;           // ダッシュ速度
    float duration_ = 0.3f;         // ダッシュ持続時間
    float cooldown_ = 1.0f;         // クールダウン時間

    float currentDuration_ = 0.0f;  // 現在のダッシュ時間
    float currentCooldown_ = 0.0f;  // 現在のクールダウン時間
    bool isDashing_ = false;        // ダッシュ中フラグ
    Vector3 dashDirection_{};       // ダッシュ方向

public:
    DashBehavior() = default;
    DashBehavior(float speed, float duration, float cooldown)
        : speed_(speed), duration_(duration), cooldown_(cooldown) {}
    ~DashBehavior() override = default;

    Vector3 Calculate(const Input* _input, float _deltaTime) override;
    bool CanExecute(const Input* _input) override;
    void Debug() const override;

    int GetPriority() const override { return 10; } // 高優先度

    // Setters
    void SetSpeed(float speed) { speed_ = speed; }
    void SetDuration(float duration) { duration_ = duration; }
    void SetCooldown(float cooldown) { cooldown_ = cooldown; }

    // Getters
    float GetSpeed() const { return speed_; }
    float GetDuration() const { return duration_; }
    float GetCooldown() const { return cooldown_; }
    bool IsDashing() const { return isDashing_; }
    float GetCooldownRemaining() const { return currentCooldown_; }

private:
    /// <summary>
    /// 入力からベクトルを取得
    /// </summary>
    Vector3 GetInputVector(const Input* _input) const;

    bool IsCooldown();
};

#endif // DASH_BEHAVIOR_HPP_
