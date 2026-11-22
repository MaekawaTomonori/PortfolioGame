#ifndef ENEMY_STATE_IDLE_HPP_
#define ENEMY_STATE_IDLE_HPP_
#include "IEnemyState.hpp"

class EnemyStateIdle : public IEnemyState {
public:
    explicit EnemyStateIdle(Enemy* _enemy) : IEnemyState(_enemy) {}

    void Enter() override;
    std::unique_ptr<IEnemyState> Update() override;
    const char* GetStateName() const override { return "Idle"; }
};

#endif // ENEMY_STATE_IDLE_HPP_
