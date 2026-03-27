#ifndef EnemyStateDeath_HPP_
#define EnemyStateDeath_HPP_
#include "IEnemyState.hpp"

class EnemyStateDeath : public IEnemyState {
public:
    explicit EnemyStateDeath(Enemy* enemy) : IEnemyState(enemy) {}

    void Enter() override;
    std::unique_ptr<IEnemyState> Update(float deltaTime) override;

    bool IsDying() const override { return true; }
    const char* GetStateName() const override { return "Death"; }

}; // class EnemyStateDeath

#endif // EnemyStateDeath_HPP_
