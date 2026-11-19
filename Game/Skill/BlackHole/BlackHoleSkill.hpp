#ifndef BLACK_HOLE_SKILL_HPP_
#define BLACK_HOLE_SKILL_HPP_

#include "BlackHole.hpp"
#include "Skill/ISkill.hpp"

class Enemies;
class ParticleSystem;

class BlackHoleSkill : public ISkill {
    BlackHole::Params params_;
    Enemies* enemies_ = nullptr;
    ParticleSystem* particle_ = nullptr;

public:
    BlackHoleSkill(Enemies* _enemies, ParticleSystem* _particle);

    std::vector<std::unique_ptr<SkillEntity>> Activate(const SkillContext& _context) override;
    float GetCooldown() const override;

    void Load() override;
    void Save() const override;
    void Debug() override;
};

#endif // BLACK_HOLE_SKILL_HPP_
