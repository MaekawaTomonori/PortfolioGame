#ifndef WALK_BEHAVIOR_HPP_
#define WALK_BEHAVIOR_HPP_

#include "IMovementBehavior.hpp"

/**
 * @brief 通常歩行動作
 */
class WalkBehavior : public IMovementBehavior {
private:
    float speed_ = 5.0f;

public:
    WalkBehavior() = default;
    explicit WalkBehavior(float speed) : speed_(speed) {}
    ~WalkBehavior() override = default;

    Vector3 Calculate(const MovementContext& context, float deltaTime) override;
    bool CanExecute(const MovementContext& context) override;
    int GetPriority() const override { return 0; }

    void SetSpeed(float speed) { speed_ = speed; }
    float GetSpeed() const { return speed_; }
};

#endif // WALK_BEHAVIOR_HPP_
