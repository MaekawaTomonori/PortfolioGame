#ifndef I_SKILL_HPP_
#define I_SKILL_HPP_

#include <memory>
#include <vector>

#include "SkillEntity.hpp"

struct SkillContext {
    GameObject& owner;
    Vector3 position;
    Vector3 direction;
};

class ISkill {
public:
    virtual ~ISkill() = default;

    /** @brief 発動して、場に出す物を返す。何も出さないスキルは空を返す */
    virtual std::vector<std::unique_ptr<SkillEntity>> Activate(const SkillContext& _context) = 0;

    virtual float GetCooldown() const = 0;

    virtual void Load() = 0;
    virtual void Save() const = 0;
    virtual void Debug() = 0;
};

#endif // I_SKILL_HPP_
