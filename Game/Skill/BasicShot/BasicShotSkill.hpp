#ifndef BASIC_SHOT_SKILL_HPP_
#define BASIC_SHOT_SKILL_HPP_

#include "Bullet/Bullet.hpp"
#include "Player/Status/PlayerStatus.hpp"
#include "Skill/ISkill.hpp"

/** @brief 狙った方向へ弾を1発撃つ。撃つ間隔は攻撃速度で短くなる */
class BasicShotSkill : public ISkill {
    const PlayerStatus& status_;

    float baseInterval_ = 1.f;
    float minInterval_ = 0.1f;
    Bullet::Params bulletParams_;

public:
    explicit BasicShotSkill(const PlayerStatus& _status);

    std::vector<std::unique_ptr<SkillEntity>> Activate(const SkillContext& _context) override;
    float GetCooldown() const override;

    void Load() override;
    void Save() const override;
    void Debug() override;
};

#endif // BASIC_SHOT_SKILL_HPP_
