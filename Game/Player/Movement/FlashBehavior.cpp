#include "FlashBehavior.hpp"

#include <algorithm>
#include "imgui.h"

Vector3 FlashBehavior::Calculate(const MovementContext& context, float deltaTime) {
    if (deltaTime <= 0.0001f) return {};

    Vector3 direction = context.targetPosition - context.position;
    direction.y = 0.0f;

    if (direction.Length() < 0.01f) {
        direction = {0, 0, 1};
    } else {
        direction = direction.Normalize();
    }

    currentCooldown_ = cooldown_;

    return direction * (distance_ / deltaTime);
}

bool FlashBehavior::CanExecute(const MovementContext& context) {
    if (currentCooldown_ > 0.0f) {
        currentCooldown_ -= 1.0f / 60.0f;
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
