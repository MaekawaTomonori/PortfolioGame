#include "SkillManager.hpp"

#include <algorithm>
#include <iterator>

#ifdef _DEBUG
#include <imgui.h>
#endif

namespace {
    bool IsValid(const SkillSlot _slot) {
        return static_cast<size_t>(_slot) < static_cast<size_t>(SkillSlot::Count);
    }
}

void SkillManager::Initialize() {
    entities_.clear();
    
    for (auto& slot : slots_) {
        slot.cooldown.Reset();
    }
}

void SkillManager::Update() {
    std::erase_if(entities_, [](const auto& _entity) { return !_entity->IsActive(); });

    for (auto& entity : entities_) {
        entity->Update();
    }

    for (auto& slot : slots_) {
        slot.cooldown.Update();
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
        
        ImGui::SeparatorText("Active Entities");
        ImGui::Text("Count: %d", static_cast<int>(entities_.size()));

        ImGui::Separator();

        for (auto& slot : slots_) {
            if (!slot.skill) continue; 
            slot.skill->Debug();
        }
    }
    ImGui::End();
#endif
}

void SkillManager::Equip(SkillSlot _slot, std::unique_ptr<ISkill> _skill) {
    if (!_skill) return;
    if (!IsValid(_slot)) return;

    auto& skill = slots_[static_cast<size_t>(_slot)].skill;
    skill = std::move(_skill);
    skill->Load();
}

void SkillManager::Activate(SkillSlot _slot, const SkillContext& _context) {
    if (!IsValid(_slot)) return;

    auto& slot = slots_[static_cast<size_t>(_slot)];

    auto& skill = slot.skill;
    auto& cooldown = slot.cooldown;

    if (!skill || !cooldown.IsReady()) return;
    auto created = skill->Activate(_context);
    cooldown.Start(skill->GetCooldown());
    
    std::ranges::move(created, std::back_inserter(entities_));
}

float SkillManager::GetCooldownRatio(SkillSlot _slot) const {
    if (!IsValid(_slot)) return 0.f;

    return slots_[static_cast<size_t>(_slot)].cooldown.GetRatio();
}
