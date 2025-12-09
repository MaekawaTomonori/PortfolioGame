#include "WalkBehavior.hpp"

Vector3 WalkBehavior::Calculate(const MovementContext& context, float deltaTime) {
    (void)deltaTime;
    return context.moveDirection * speed_;
}

bool WalkBehavior::CanExecute(const MovementContext& context) {
    return context.moveDirection.Length() > 0.0001f;
}
