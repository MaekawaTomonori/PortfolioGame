#ifndef EnemyStateCharging_HPP_
#define EnemyStateCharging_HPP_
#include "IEnemyState.hpp"
#include "Math/Vector3.hpp"
#include <memory>

class Model;

class EnemyStateCharging : public IEnemyState {
    Vector3 dashDirection_;
    std::unique_ptr<Model> prediction_;

public:
    EnemyStateCharging(Enemy* enemy, const Vector3& dashDirection);
    ~EnemyStateCharging() override = default;

    void Enter() override;
    std::unique_ptr<IEnemyState> Update(float deltaTime) override;
    void Exit() override;
    void Draw() override;
    const char* GetStateName() const override { return "Charging"; }

private:
    void UpdatePredictionLine();

}; // class EnemyStateCharging

#endif // EnemyStateCharging_HPP_
