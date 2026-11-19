#ifndef SKILL_ENTITY_HPP_
#define SKILL_ENTITY_HPP_

#include "GameObject/GameObject.hpp"
#include "Collision/Collider.h"

#include <memory>

/** @brief スキルエンティティの基底クラス
 ** GameObjectを継承し、コライダーとライフタイムを共通で管理する
 **/
class SkillEntity : public GameObject {
protected:
    std::unique_ptr<Collision::Collider> collider_;
    float lifetime_ = 5.f;

public:
    ~SkillEntity() override = default;

    /** @brief スキルエンティティを初期化
     ** @param _position 発生位置
     ** @param _direction 発射方向
     **/
    virtual void Initialize(const Vector3& _position, const Vector3& _direction) = 0;

    void Initialize() override {}
};

#endif // SKILL_ENTITY_HPP_
