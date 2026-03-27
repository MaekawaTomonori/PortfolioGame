#ifndef EnemyStateIdle_HPP_
#define EnemyStateIdle_HPP_
#include "IEnemyState.hpp"

class EnemyStateIdle : public IEnemyState {
    float triggerTimer_ = 0.f;

public:
    explicit EnemyStateIdle(Enemy* _enemy) : IEnemyState(_enemy) {}

    void Enter() override;
    std::unique_ptr<IEnemyState> Update(float _deltaTime) override;
    const char* GetStateName() const override { return "Idle"; }
};

#endif // EnemyStateIdle_HPP_
