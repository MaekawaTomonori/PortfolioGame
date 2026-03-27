#ifndef IENEMYSTATE_HPP
#define IENEMYSTATE_HPP
#include <memory>

class Enemy;

class IEnemyState {
protected:
    Enemy* enemy_ = nullptr;
    float elapsedTime_ = 0.f;

public:
    explicit IEnemyState(Enemy* enemy) : enemy_(enemy) {}
    virtual ~IEnemyState() = default;

    virtual void Enter() {}
    virtual std::unique_ptr<IEnemyState> Update(float deltaTime) = 0;
    virtual void Exit() {}
    virtual void Draw() {}
    virtual bool IsDying() const { return false; }
    virtual const char* GetStateName() const { return "Unknown"; }
};

#endif // IENEMYSTATE_HPP
