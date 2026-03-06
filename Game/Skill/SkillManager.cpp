#include "SkillManager.hpp"

#include "Enemy/Enemies.hpp"

#ifdef _DEBUG
#include <imgui.h>
#endif

void SkillManager::Initialize() {
    entities_.clear();
}

void SkillManager::Update() {
    std::erase_if(entities_, [](const auto& _entity) { return !_entity->IsActive(); });

    for (auto& entity : entities_) {
        entity->Update(1.f / 60.f);
    }
}

void SkillManager::Draw() const {
    for (const auto& entity : entities_) {
        entity->Draw();
    }
}

void SkillManager::Debug() {
#ifdef _DEBUG
    if (ImGui::Begin("Skill Manager")) {
        if (ImGui::CollapsingHeader("BlackHole Params", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SeparatorText("Flying");
            ImGui::DragFloat("Initial Speed", &blackHoleParams_.initialSpeed, 0.1f, 0.f, 50.f);
            ImGui::DragFloat("Deceleration", &blackHoleParams_.deceleration, 0.1f, 0.f, 30.f);
            ImGui::DragFloat("Stop Threshold", &blackHoleParams_.stopThreshold, 0.01f, 0.f, 5.f);
            ImGui::DragFloat3("Flying Scale", &blackHoleParams_.flyingScale.x, 0.01f, 0.01f, 5.f);
            ImGui::ColorEdit4("Flying Color", &blackHoleParams_.flyingColor.x);

            ImGui::SeparatorText("Sucking");
            ImGui::DragFloat("Suck Radius", &blackHoleParams_.suckRadius, 0.1f, 0.f, 30.f);
            ImGui::DragFloat("Suck Force", &blackHoleParams_.suckForce, 0.1f, 0.f, 30.f);
            ImGui::DragFloat("Suck Duration", &blackHoleParams_.suckDuration, 0.1f, 0.f, 15.f);
            ImGui::DragFloat3("Sucking Scale", &blackHoleParams_.suckingScale.x, 0.01f, 0.01f, 5.f);
            ImGui::ColorEdit4("Sucking Color", &blackHoleParams_.suckingColor.x);

            ImGui::SeparatorText("Vortex (Ground Plane)");
            ImGui::DragFloat("Rotate Speed", &blackHoleParams_.vortexRotateSpeed, 0.1f, 0.f, 20.f);
            ImGui::DragFloat3("Vortex Scale", &blackHoleParams_.vortexScale.x, 0.1f, 0.1f, 20.f);
            ImGui::ColorEdit4("Vortex Color", &blackHoleParams_.vortexColor.x);
        }

        ImGui::SeparatorText("Active Entities");
        ImGui::Text("Count: %d", static_cast<int>(entities_.size()));
    }
    ImGui::End();
#endif
}

void SkillManager::SpawnBlackHole(const Vector3& _position, const Vector3& _direction) {
    auto blackHole = std::make_unique<BlackHole>();
    blackHole->SetParams(&blackHoleParams_);
    blackHole->SetEnemies(enemies_);
    blackHole->SetParticle(particle_);
    blackHole->Initialize(_position, _direction);
    entities_.emplace_back(std::move(blackHole));
}
