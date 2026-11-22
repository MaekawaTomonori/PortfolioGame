#ifndef ENEMY_STATE_DEATH_HPP_
#define ENEMY_STATE_DEATH_HPP_
#include "IEnemyState.hpp"

class EnemyStateDeath : public IEnemyState {
public:
    explicit EnemyStateDeath(Enemy* enemy) : IEnemyState(enemy) {}

    void Enter() override;
    std::unique_ptr<IEnemyState> Update() override;

    bool IsDying() const override { return true; }
    const char* GetStateName() const override { return "Death"; }

}; // class EnemyStateDeath

#endif // ENEMY_STATE_DEATH_HPP_
