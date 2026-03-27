#ifndef EnemyStateDash_HPP_
#define EnemyStateDash_HPP_
#include "IEnemyState.hpp"
#include "Math/Vector3.hpp"

class EnemyStateDash : public IEnemyState {
    Vector3 dashDirection_;

public:
    EnemyStateDash(Enemy* _enemy, const Vector3& _dashDirection);

    void Enter() override;
    std::unique_ptr<IEnemyState> Update(float _deltaTime) override;
    void Exit() override;
    const char* GetStateName() const override { return "Dashing"; }
};

#endif // EnemyStateDash_HPP_
