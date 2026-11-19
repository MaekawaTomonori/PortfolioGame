#include "FlashSkill.hpp"

#include "Skill/SkillParamsJson.hpp"

#ifdef _DEBUG
#include "imgui.h"
#endif

std::vector<std::unique_ptr<SkillEntity>> FlashSkill::Activate(const SkillContext& _context) {
    _context.owner.SetPosition(_context.position + _context.direction * distance_);
    return {};
}

float FlashSkill::GetCooldown() const {
    return cooldown_;
}

void FlashSkill::Load() {
    if (!Singleton<JsonParams>::GetInstance()->Load(SKILL_PARAMS_FILE)) return;

    ReadSkillParam("Flash", "Distance", distance_);
    ReadSkillParam("Flash", "Cooldown", cooldown_);
}

void FlashSkill::Save() const {
    const auto& json = Singleton<JsonParams>::GetInstance();

    json->SetValue(SKILL_PARAMS_FILE, "Flash", "Distance", distance_);
    json->SetValue(SKILL_PARAMS_FILE, "Flash", "Cooldown", cooldown_);

    json->Save(SKILL_PARAMS_FILE);
}

void FlashSkill::Debug() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Flash Params", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Distance", &distance_, 0.1f, 0.f, 20.f);
        ImGui::DragFloat("Cooldown", &cooldown_, 0.1f, 0.f, 30.f);

        if (ImGui::Button("Save Flash", ImVec2(-1, 0))) Save();
    }
#endif
}
