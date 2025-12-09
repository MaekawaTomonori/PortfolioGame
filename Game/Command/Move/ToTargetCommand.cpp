#include "ToTargetCommand.hpp"
#include "imgui.h"
#include "GameObject/GameObject.hpp"

ToTargetCommand::ToTargetCommand(float speed, float minDistance)
    : speed_(speed)
    , minDistance_(minDistance) {
}

void ToTargetCommand::Execute(MovementContext& context) {
    if (!context.target) {
        return;
    }

    Vector3 targetPos = context.target->GetPosition();
    Vector3 direction = targetPos - context.position;
    float distance = direction.Length();

    if (distance <= minDistance_) {
        context.moveDirection = {};
        return;
    }

    direction.y = 0;
    context.moveDirection = direction.Normalize();
}

void ToTargetCommand::Debug() {
    ImGui::Text("Type: ToTarget (Linear)");
    ImGui::Text("Speed: %.2f", speed_);
    ImGui::Text("Min Distance: %.2f", minDistance_);
}
