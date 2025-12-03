#ifndef WALK_BEHAVIOR_HPP_
#define WALK_BEHAVIOR_HPP_

#include "IMovementBehavior.hpp"
#include "Input.hpp"

/// <summary>
/// 通常歩行動作
/// WASDキーによる基本移動を実装
/// </summary>
class WalkBehavior : public IMovementBehavior {
private:
    float speed_ = 5.0f;

public:
    WalkBehavior() = default;
    explicit WalkBehavior(float speed) : speed_(speed) {}
    ~WalkBehavior() override = default;

    Vector3 Calculate(const Input* input, float deltaTime) override;
    bool CanExecute(const Input* input) override;
    int GetPriority() const override { return 0; } // 最低優先度

    // Setters
    void SetSpeed(float speed) { speed_ = speed; }

    // Getters
    float GetSpeed() const { return speed_; }

private:
    /// <summary>
    /// 入力からベクトルを取得
    /// </summary>
    Vector3 GetInputVector(const Input* input) const;
};

#endif // WALK_BEHAVIOR_HPP_
