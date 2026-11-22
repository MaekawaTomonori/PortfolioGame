#ifndef SKILL_ENTITY_HPP_
#define SKILL_ENTITY_HPP_

#include "GameObject/GameObject.hpp"
#include "Collision/Collider.h"

#include <memory>

/** @brief スキルエンティティの基底クラス
 * GameObjectを継承し、コライダーとライフタイムを共通で管理する
 */
class SkillEntity : public GameObject {
protected:
    std::unique_ptr<Collision::Collider> collider_;
    float lifetime_ = 5.f;

public:
    ~SkillEntity() override = default;
};

#endif // SKILL_ENTITY_HPP_
