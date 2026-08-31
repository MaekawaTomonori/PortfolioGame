#ifndef I_ENEMY_ACTION_MODULE_HPP_
#define I_ENEMY_ACTION_MODULE_HPP_

class Enemy;

class IEnemyActionModule {
public:
    virtual ~IEnemyActionModule() = default;

    virtual void Update(Enemy* _enemy) = 0;
    virtual void Draw() {}
    virtual bool IsBusy() const { return false; }
};

#endif // I_ENEMY_ACTION_MODULE_HPP_
