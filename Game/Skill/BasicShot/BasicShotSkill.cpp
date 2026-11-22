#include "BasicShotSkill.hpp"

#include <algorithm>

#include "Skill/SkillParamsJson.hpp"

#ifdef _DEBUG
#include "imgui.h"
#endif

BasicShotSkill::BasicShotSkill(const PlayerStatus& _status)
    : status_(_status) {}

std::vector<std::unique_ptr<SkillEntity>> BasicShotSkill::Activate(const SkillContext& _context) {
    Bullet::Params params = bulletParams_;
    params.damage = status_.damage;

    std::vector<std::unique_ptr<SkillEntity>> spawned;
    spawned.emplace_back(std::make_unique<Bullet>(params, _context.position, _context.direction));
    return spawned;
}

float BasicShotSkill::GetCooldown() const {
    if (status_.as <= 0.f) return baseInterval_;
    return std::max(minInterval_, baseInterval_ / status_.as);
}

void BasicShotSkill::Load() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load(SKILL_PARAMS_FILE)) return;

    json->TryGetValue(SKILL_PARAMS_FILE, "BasicShot", "Interval", baseInterval_);
    json->TryGetValue(SKILL_PARAMS_FILE, "BasicShot", "MinInterval", minInterval_);
    json->TryGetValue(SKILL_PARAMS_FILE, "BasicShot", "Speed", bulletParams_.speed);
    json->TryGetValue(SKILL_PARAMS_FILE, "BasicShot", "Lifetime", bulletParams_.lifetime);
}

void BasicShotSkill::Save() const {
    const auto& json = Singleton<JsonParams>::GetInstance();

    json->SetValue(SKILL_PARAMS_FILE, "BasicShot", "Interval", baseInterval_);
    json->SetValue(SKILL_PARAMS_FILE, "BasicShot", "MinInterval", minInterval_);
    json->SetValue(SKILL_PARAMS_FILE, "BasicShot", "Speed", bulletParams_.speed);
    json->SetValue(SKILL_PARAMS_FILE, "BasicShot", "Lifetime", bulletParams_.lifetime);

    json->Save(SKILL_PARAMS_FILE);
}

void BasicShotSkill::Debug() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("BasicShot Params", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Interval", &baseInterval_, 0.01f, 0.f, 5.f);
        ImGui::DragFloat("Min Interval", &minInterval_, 0.01f, 0.f, 5.f);
        ImGui::DragFloat("Bullet Speed", &bulletParams_.speed, 0.1f, 0.f, 50.f);
        ImGui::DragFloat("Bullet Lifetime", &bulletParams_.lifetime, 0.1f, 0.f, 10.f);

        if (ImGui::Button("Save BasicShot", ImVec2(-1, 0))) Save();
    }
#endif
}
