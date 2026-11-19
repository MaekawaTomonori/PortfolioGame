#ifndef FLASH_SKILL_HPP_
#define FLASH_SKILL_HPP_

#include "Skill/ISkill.hpp"

/** @brief 向いている方向へ一瞬で移動する */
class FlashSkill : public ISkill {
    float distance_ = 5.f;
    float cooldown_ = 5.f;

public:
    std::vector<std::unique_ptr<SkillEntity>> Activate(const SkillContext& _context) override;
    float GetCooldown() const override;

    void Load() override;
    void Save() const override;
    void Debug() override;
};

#endif // FLASH_SKILL_HPP_
