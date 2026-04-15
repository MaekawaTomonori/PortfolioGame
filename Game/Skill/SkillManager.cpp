#include "SkillManager.hpp"

#include "Enemy/Enemies.hpp"
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"

#ifdef _DEBUG
#include <imgui.h>
#endif

void SkillManager::Initialize() {
    entities_.clear();
    LoadParams();
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

        ImGui::Separator();
        if (ImGui::Button("Save to JSON", ImVec2(-1, 0))) SaveParams();
        ImGui::TextDisabled("Assets/Data/SkillParams/SkillParams.json");

        ImGui::SeparatorText("Active Entities");
        ImGui::Text("Count: %d", static_cast<int>(entities_.size()));
    }
    ImGui::End();
#endif
}

void SkillManager::LoadParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("SkillParams")) return;

    blackHoleParams_.initialSpeed     = std::get<float>  (json->GetValue("SkillParams", "BlackHole_Flying",  "InitialSpeed"));
    blackHoleParams_.deceleration     = std::get<float>  (json->GetValue("SkillParams", "BlackHole_Flying",  "Deceleration"));
    blackHoleParams_.stopThreshold    = std::get<float>  (json->GetValue("SkillParams", "BlackHole_Flying",  "StopThreshold"));
    blackHoleParams_.flyingScale      = std::get<Vector3>(json->GetValue("SkillParams", "BlackHole_Flying",  "FlyingScale"));
    blackHoleParams_.flyingColor      = std::get<Vector4>(json->GetValue("SkillParams", "BlackHole_Flying",  "FlyingColor"));

    blackHoleParams_.suckRadius       = std::get<float>  (json->GetValue("SkillParams", "BlackHole_Sucking", "SuckRadius"));
    blackHoleParams_.suckForce        = std::get<float>  (json->GetValue("SkillParams", "BlackHole_Sucking", "SuckForce"));
    blackHoleParams_.suckDuration     = std::get<float>  (json->GetValue("SkillParams", "BlackHole_Sucking", "SuckDuration"));
    blackHoleParams_.suckingScale     = std::get<Vector3>(json->GetValue("SkillParams", "BlackHole_Sucking", "SuckingScale"));
    blackHoleParams_.suckingColor     = std::get<Vector4>(json->GetValue("SkillParams", "BlackHole_Sucking", "SuckingColor"));

    blackHoleParams_.vortexRotateSpeed= std::get<float>  (json->GetValue("SkillParams", "BlackHole_Vortex",  "VortexRotateSpeed"));
    blackHoleParams_.vortexScale      = std::get<Vector3>(json->GetValue("SkillParams", "BlackHole_Vortex",  "VortexScale"));
    blackHoleParams_.vortexColor      = std::get<Vector4>(json->GetValue("SkillParams", "BlackHole_Vortex",  "VortexColor"));
}

void SkillManager::SaveParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();

    json->SetValue("SkillParams", "BlackHole_Flying",  "InitialSpeed",     blackHoleParams_.initialSpeed);
    json->SetValue("SkillParams", "BlackHole_Flying",  "Deceleration",     blackHoleParams_.deceleration);
    json->SetValue("SkillParams", "BlackHole_Flying",  "StopThreshold",    blackHoleParams_.stopThreshold);
    json->SetValue("SkillParams", "BlackHole_Flying",  "FlyingScale",      blackHoleParams_.flyingScale);
    json->SetValue("SkillParams", "BlackHole_Flying",  "FlyingColor",      blackHoleParams_.flyingColor);

    json->SetValue("SkillParams", "BlackHole_Sucking", "SuckRadius",       blackHoleParams_.suckRadius);
    json->SetValue("SkillParams", "BlackHole_Sucking", "SuckForce",        blackHoleParams_.suckForce);
    json->SetValue("SkillParams", "BlackHole_Sucking", "SuckDuration",     blackHoleParams_.suckDuration);
    json->SetValue("SkillParams", "BlackHole_Sucking", "SuckingScale",     blackHoleParams_.suckingScale);
    json->SetValue("SkillParams", "BlackHole_Sucking", "SuckingColor",     blackHoleParams_.suckingColor);

    json->SetValue("SkillParams", "BlackHole_Vortex",  "VortexRotateSpeed",blackHoleParams_.vortexRotateSpeed);
    json->SetValue("SkillParams", "BlackHole_Vortex",  "VortexScale",      blackHoleParams_.vortexScale);
    json->SetValue("SkillParams", "BlackHole_Vortex",  "VortexColor",      blackHoleParams_.vortexColor);

    json->Save("SkillParams");
}

void SkillManager::SpawnBlackHole(const Vector3& _position, const Vector3& _direction) {
    auto blackHole = std::make_unique<BlackHole>();
    blackHole->SetParams(&blackHoleParams_);
    blackHole->SetEnemies(enemies_);
    blackHole->SetParticle(particle_);
    blackHole->Initialize(_position, _direction);
    entities_.emplace_back(std::move(blackHole));
}
