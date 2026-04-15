#include "FlashBehavior.hpp"

#include <algorithm>
#include "imgui.h"

namespace {
    constexpr float DELTA_TIME = 1.f / 60.f;
    constexpr float MIN_DELTA_TIME = 0.0001f;
    constexpr float MIN_DIRECTION_LENGTH = 0.01f;
}

Vector3 FlashBehavior::Calculate(const MovementContext& context, float deltaTime) {
    if (deltaTime <= MIN_DELTA_TIME) return {};

    Vector3 direction = context.targetPosition - context.position;
    direction.y = 0.0f;

    if (direction.Length() < MIN_DIRECTION_LENGTH) {
        direction = {0, 0, 1};
    } else {
        direction = direction.Normalize();
    }

    currentCooldown_ = cooldown_;

    return direction * (distance_ / deltaTime);
}

bool FlashBehavior::CanExecute(const MovementContext& context) {
    if (currentCooldown_ > 0.0f) {
        currentCooldown_ -= DELTA_TIME;
        currentCooldown_ = std::max(currentCooldown_, 0.0f);
    }

    if (context.isFlashRequested && currentCooldown_ <= 0.0f) {
        return true;
    }
    return false;
}

void FlashBehavior::Debug() const {
    ImGui::Text("Flash Cooldown: %.2f", currentCooldown_);
}
