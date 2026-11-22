#include "BlackHoleSkill.hpp"

#include "Skill/SkillParamsJson.hpp"

#ifdef _DEBUG
#include "imgui.h"
#endif

BlackHoleSkill::BlackHoleSkill(Enemies* _enemies, ParticleSystem* _particle)
    : enemies_(_enemies), particle_(_particle) {}

std::vector<std::unique_ptr<SkillEntity>> BlackHoleSkill::Activate(const SkillContext& _context) {
    auto blackHole = std::make_unique<BlackHole>(params_, _context.position, _context.direction);
    blackHole->SetEnemies(enemies_);
    blackHole->SetParticle(particle_);

    std::vector<std::unique_ptr<SkillEntity>> spawned;
    spawned.emplace_back(std::move(blackHole));
    return spawned;
}

float BlackHoleSkill::GetCooldown() const {
    return params_.cooldownDuration;
}

void BlackHoleSkill::Load() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load(SKILL_PARAMS_FILE)) return;

    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "InitialSpeed", params_.initialSpeed);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "Deceleration", params_.deceleration);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "StopThreshold", params_.stopThreshold);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "FlyingScale", params_.flyingScale);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "FlyingColor", params_.flyingColor);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckRadius", params_.suckRadius);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckForce", params_.suckForce);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckDuration", params_.suckDuration);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckingScale", params_.suckingScale);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckingColor", params_.suckingColor);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Vortex", "VortexRotateSpeed", params_.vortexRotateSpeed);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Vortex", "VortexScale", params_.vortexScale);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Vortex", "VortexColor", params_.vortexColor);
    json->TryGetValue(SKILL_PARAMS_FILE, "BlackHole_Cooldown", "CooldownDuration", params_.cooldownDuration);
}

void BlackHoleSkill::Save() const {
    const auto& json = Singleton<JsonParams>::GetInstance();

    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "InitialSpeed", params_.initialSpeed);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "Deceleration", params_.deceleration);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "StopThreshold", params_.stopThreshold);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "FlyingScale", params_.flyingScale);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Flying", "FlyingColor", params_.flyingColor);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckRadius", params_.suckRadius);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckForce", params_.suckForce);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckDuration", params_.suckDuration);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckingScale", params_.suckingScale);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Sucking", "SuckingColor", params_.suckingColor);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Vortex", "VortexRotateSpeed", params_.vortexRotateSpeed);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Vortex", "VortexScale", params_.vortexScale);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Vortex", "VortexColor", params_.vortexColor);
    json->SetValue(SKILL_PARAMS_FILE, "BlackHole_Cooldown", "CooldownDuration", params_.cooldownDuration);

    json->Save(SKILL_PARAMS_FILE);
}

void BlackHoleSkill::Debug() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("BlackHole Params", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SeparatorText("Flying");
        ImGui::DragFloat("Initial Speed", &params_.initialSpeed, 0.1f, 0.f, 50.f);
        ImGui::DragFloat("Deceleration", &params_.deceleration, 0.1f, 0.f, 30.f);
        ImGui::DragFloat("Stop Threshold", &params_.stopThreshold, 0.01f, 0.f, 5.f);
        ImGui::DragFloat3("Flying Scale", &params_.flyingScale.x, 0.01f, 0.01f, 5.f);
        ImGui::ColorEdit4("Flying Color", &params_.flyingColor.x);

        ImGui::SeparatorText("Sucking");
        ImGui::DragFloat("Suck Radius", &params_.suckRadius, 0.1f, 0.f, 30.f);
        ImGui::DragFloat("Suck Force", &params_.suckForce, 0.1f, 0.f, 30.f);
        ImGui::DragFloat("Suck Duration", &params_.suckDuration, 0.1f, 0.f, 15.f);
        ImGui::DragFloat3("Sucking Scale", &params_.suckingScale.x, 0.01f, 0.01f, 5.f);
        ImGui::ColorEdit4("Sucking Color", &params_.suckingColor.x);

        ImGui::SeparatorText("Vortex (Ground Plane)");
        ImGui::DragFloat("Rotate Speed", &params_.vortexRotateSpeed, 0.1f, 0.f, 20.f);
        ImGui::DragFloat3("Vortex Scale", &params_.vortexScale.x, 0.1f, 0.1f, 20.f);
        ImGui::ColorEdit4("Vortex Color", &params_.vortexColor.x);

        ImGui::SeparatorText("Cooldown");
        ImGui::DragFloat("Cooldown Duration", &params_.cooldownDuration, 0.1f, 0.f, 30.f);
    }

    ImGui::Separator();
    if (ImGui::Button("Save to JSON", ImVec2(-1, 0))) Save();
    ImGui::TextDisabled("Assets/Data/SkillParams/SkillParams.json");
#endif
}
