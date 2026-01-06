#ifndef Enemies_HPP_
#define Enemies_HPP_
#include <vector>
#include <memory>

#include "Enemy.hpp"
#include "ParticleSystem/ParticleSystem.hpp"
#include "Player/Movement/IMovementBehavior.hpp"
#include "Command/ICommand.hpp"

class Enemies {
    ParticleSystem* particle_ = nullptr;

    GameObject* target_ = nullptr;
    std::vector<std::unique_ptr<Enemy>> enemies_;

    // 共有Behavior（Flyweight Pattern）
    std::unique_ptr<IMovementBehavior> walkBehavior_;
    std::unique_ptr<IMovementBehavior> dashBehavior_;

    // 共有Command（Flyweight Pattern）
    std::unique_ptr<ICommand> toTargetCommand_;

    const uint16_t MaxEnemies = 3;
    const float Interval = 2.f;
    float timer_ = 0.f;

    Vector2 distance_ = { 5.f, 10.f };

    // 敵の共通パラメータ
    Enemy::Params enemyParams_;

#ifdef _DEBUG
    bool autoSpawn_ = true;
    Line line_;
#endif

public:
    void Initialize(ParticleSystem* _particle);
    void Update();
    void Draw() const;

    [[nodiscard]] Vector3 GetNearest(Vector3 _pos) const;

    void SetTarget(GameObject* _target);

    void Debug();

    bool Empty() const;

    // カメラ調整用の情報取得
    [[nodiscard]] float GetFarthestEnemyDistance(Vector3 referencePos) const;

private:
    void Spawn();
    void LoadParams();
    void SaveParams();

}; // class Enemies

#endif // Enemies_HPP_
