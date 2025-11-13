#ifndef MOVEMENT_COMPONENT_HPP_
#define MOVEMENT_COMPONENT_HPP_

#include "Input.hpp"
#include "Math/Vector3.hpp"

class GameObject;

class MovementComponent {
private:
    float moveSpeed_{5.0f};
    Input* input_;
    GameObject* owner_;

public:
    MovementComponent();
    ~MovementComponent();

    void Initialize(GameObject* owner);
    void Update();

    // Getters
    float GetMoveSpeed() const { return moveSpeed_; }

    // Setters
    void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

private:
    Vector3 GetInputVector() const;
};

#endif // MOVEMENT_COMPONENT_HPP_