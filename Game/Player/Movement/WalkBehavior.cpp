#include "WalkBehavior.hpp"

namespace {
    constexpr float MIN_MOVE_THRESHOLD = 0.0001f;
}

Vector3 WalkBehavior::Calculate(const MovementContext& context, float deltaTime) {
    (void)deltaTime;
    return context.moveDirection * speed_;
}

bool WalkBehavior::CanExecute(const MovementContext& context) {
    return context.moveDirection.Length() > MIN_MOVE_THRESHOLD;
}
