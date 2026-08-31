#ifndef DASH_ACTION_MODULE_HPP_
#define DASH_ACTION_MODULE_HPP_

#include <memory>
#include "IEnemyActionModule.hpp"
#include "Math/Vector3.hpp"

class Model;

class DashActionModule : public IEnemyActionModule {
public:
    struct Params {
        float chargeMaxTime = 2.0f;
        float duration = 0.4f;
        float speed = 20.0f;
        float triggerInterval = 3.0f;
        float triggerChance = 0.3f;
        float enemyOffset = 1.5f;
        float lineLength = 0.7f;
        float lineWidth = 1.5f;
        float lineCenterOffset = 0.6f;
    };

private:
    enum class Phase { Waiting, Charging, Dashing };

    const Params* params_ = nullptr;

    Phase phase_ = Phase::Waiting;
    float triggerTimer_ = 0.f;
    float elapsedTime_ = 0.f;
    Vector3 dashDirection_ {};

    std::unique_ptr<Model> prediction_;

public:
    void SetParams(const Params* _params) { params_ = _params; }

    void Update(Enemy* _enemy) override;
    void Draw() override;
    bool IsBusy() const override { return phase_ != Phase::Waiting; }

private:
    void UpdateWaiting(Enemy* _enemy);
    void EnterCharging(Enemy* _enemy);
    void UpdateCharging(Enemy* _enemy);
    void EnterDashing(Enemy* _enemy);
    void UpdateDashing(Enemy* _enemy);
    void UpdatePredictionLine();
};

#endif // DASH_ACTION_MODULE_HPP_
