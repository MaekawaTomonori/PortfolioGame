#include "DashBehavior.hpp"
#include "imgui.h"

Vector3 DashBehavior::Calculate(const MovementContext& context, const float deltaTime) {
    if (!isDashing_ && CanExecute(context)) {
        isDashing_ = true;
        currentDuration_ = duration_;
        dashDirection_ = context.moveDirection;
        if (dashDirection_.Length() <= 0.0001f) {
             isDashing_ = false;
             return {};
        }
        dashDirection_ = dashDirection_.Normalize();
    }

    if (isDashing_) {
        currentDuration_ -= deltaTime;

        if (currentDuration_ <= 0.0f) {
            isDashing_ = false;
            currentCooldown_ = cooldown_;
            currentDuration_ = 0.0f;
            return {};
        }

        return dashDirection_ * speed_;
    }

    return {};
}

bool DashBehavior::CanExecute(const MovementContext& context) {
    if (IsCooldown()) return false;

    if (isDashing_) return true;

    bool hasDashInput = context.isDashRequested;
    bool hasMovement = context.moveDirection.Length() > 0.0001f;

    return hasDashInput && hasMovement;
}

void DashBehavior::Debug() const {
    ImGui::Begin("DashBehavior Debug");
    ImGui::Text("Speed: %f", speed_);
    ImGui::Text("Duration: %f", duration_);
    ImGui::Text("Cooldown: %f", cooldown_);
    ImGui::Text("Is Dashing: %s", isDashing_ ? "Yes" : "No");
    ImGui::Text("Cooldown Remaining: %f", currentCooldown_);
    ImGui::ProgressBar((cooldown_ - currentCooldown_) / cooldown_, ImVec2(-1.0f, 0.0f));
    ImGui::End();
}

bool DashBehavior::IsCooldown() {
    if (currentCooldown_ <= 0.f) {
        currentCooldown_ = 0.f;
        return false;
    }

    currentCooldown_ -= 1.f / 60.f;
    return true;
}
