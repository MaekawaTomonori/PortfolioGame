#ifndef RANGED_ACTION_MODULE_HPP_
#define RANGED_ACTION_MODULE_HPP_

#include <functional>
#include "IEnemyActionModule.hpp"
#include "Bullet/Bullet.hpp"
#include "Math/Vector3.hpp"

class RangedActionModule : public IEnemyActionModule {
public:
    struct Params {
        float triggerInterval = 2.5f;
        float triggerChance = 0.5f;
        float minRange = 0.f;
        float maxRange = 15.f;
        Bullet::Params bullet;
    };

private:
    const Params* params_ = nullptr;
    float triggerTimer_ = 0.f;
    std::function<void(Enemy*, const Vector3&)> onFire_;

public:
    void SetParams(const Params* _params) { params_ = _params; }
    void SetOnFire(std::function<void(Enemy*, const Vector3&)> _callback) { onFire_ = std::move(_callback); }

    void Update(Enemy* _enemy) override;
};

#endif // RANGED_ACTION_MODULE_HPP_
