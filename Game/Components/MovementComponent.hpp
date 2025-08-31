#ifndef MOVEMENT_COMPONENT_HPP_
#define MOVEMENT_COMPONENT_HPP_

#include "Input.hpp"
#include "Math/Vector3.hpp"
#include "Pattern/Singleton.hpp"

class MovementComponent {
private:
    Vector3 velocity_{0.0f, 0.0f, 0.0f};
    float moveSpeed_{5.0f};
    Input* input_;

public:
    MovementComponent();
    ~MovementComponent();
    
    void Initialize();
    Vector3 Update(float deltaTime);
    
    // Getters
    const Vector3& GetVelocity() const { return velocity_; }
    float GetMoveSpeed() const { return moveSpeed_; }
    
    // Setters
    void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

private:
    Vector3 GetInputVector() const;
};

#endif // MOVEMENT_COMPONENT_HPP_