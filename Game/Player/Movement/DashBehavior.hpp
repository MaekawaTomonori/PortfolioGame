#ifndef DASH_BEHAVIOR_HPP_
#define DASH_BEHAVIOR_HPP_

#include "IMovementBehavior.hpp"

/**
 * @brief ダッシュ動作
 */
class DashBehavior : public IMovementBehavior {
    float speed_ = 10.0f;
    float duration_ = 0.3f;
    float cooldown_ = 1.0f;

    float currentDuration_ = 0.0f;
    float currentCooldown_ = 0.0f;
    bool isDashing_ = false;
    Vector3 dashDirection_{};

public:
    DashBehavior() = default;
    DashBehavior(float speed, float duration, float cooldown)
        : speed_(speed), duration_(duration), cooldown_(cooldown) {}
    ~DashBehavior() override = default;

    Vector3 Calculate(const MovementContext& context, float deltaTime) override;
    bool CanExecute(const MovementContext& context) override;
    void Debug() const override;

    int GetPriority() const override { return 10; }

    void SetSpeed(float speed) { speed_ = speed; }
    void SetDuration(float duration) { duration_ = duration; }
    void SetCooldown(float cooldown) { cooldown_ = cooldown; }

    float GetSpeed() const { return speed_; }
    float GetDuration() const { return duration_; }
    float GetCooldown() const { return cooldown_; }
    bool IsDashing() const { return isDashing_; }
    float GetCooldownRemaining() const { return currentCooldown_; }

private:
    bool IsCooldown();
};

#endif // DASH_BEHAVIOR_HPP_
