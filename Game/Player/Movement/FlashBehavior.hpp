#ifndef FLASH_BEHAVIOR_HPP_
#define FLASH_BEHAVIOR_HPP_

#include "IMovementBehavior.hpp"

/**
 * @brief フラッシュ（瞬間移動）動作
 */
class FlashBehavior : public IMovementBehavior {
    float distance_ = 5.0f;
    float cooldown_ = 5.0f;
    float currentCooldown_ = 0.0f;

public:
    FlashBehavior() = default;
    explicit FlashBehavior(float distance, float cooldown)
        : distance_(distance), cooldown_(cooldown) {}

    Vector3 Calculate(const MovementContext& context, float deltaTime) override;
    bool CanExecute(const MovementContext& context) override;
    int GetPriority() const override { return 100; }
    void Debug() const override;

    void SetDistance(float distance) { distance_ = distance; }
    void SetCooldown(float cooldown) { cooldown_ = cooldown; }
    float GetDistance() const { return distance_; }
    float GetCooldown() const { return cooldown_; }

private:
    bool IsCooldown();
};

#endif // FLASH_BEHAVIOR_HPP_
