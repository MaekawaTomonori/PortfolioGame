#ifndef SKILL_MANAGER_HPP_
#define SKILL_MANAGER_HPP_

#include <array>
#include <memory>
#include <vector>

#include "ISkill.hpp"
#include "SkillEntity.hpp"
#include "SkillSlot.hpp"
#include "Combat/Cooldown.hpp"

class SkillManager {

    struct Slot {
        std::unique_ptr<ISkill> skill;
        Cooldown cooldown;
    };

    std::vector<std::unique_ptr<SkillEntity>> entities_;

    std::array<Slot, static_cast<size_t>(SkillSlot::Count)> slots_;

public:
    void Initialize();
    void Update();
    void Draw() const;
    void Debug();

    void Equip(SkillSlot _slot, std::unique_ptr<ISkill> _skill);

    /** @brief スキル発動
     * @param _slot どのスロットのスキルか
     * @param _context 位置や発動方向を指定
     */
    void Activate(SkillSlot _slot, const SkillContext& _context);

    float GetCooldownRatio(SkillSlot _slot) const;
};

#endif // SKILL_MANAGER_HPP_
