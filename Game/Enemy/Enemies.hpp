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

    std::unique_ptr<IMovementBehavior> walkBehavior_;
    std::unique_ptr<IMovementBehavior> dashBehavior_;

    std::unique_ptr<ICommand> toTargetCommand_;

    uint16_t maxCount_ = 3;
    float interval_ = 2.f;
    float timer_ = 0.f;
    uint16_t deathCount_ = 0;

    Vector2 distance_ = { 5.f, 10.f };

    // 敵の共通パラメータ
    Enemy::Params enemyParams_;

#ifdef _DEBUG
    bool autoSpawn_ = true;
    Line line_;
#endif

public:
    Enemies(ParticleSystem* _particle);
    void Initialize();
    void Update();
    void Draw() const;

    [[nodiscard]] Vector3 GetNearest(Vector3 _pos) const;

    void SetTarget(GameObject* _target);
    void SetMaxCount(uint16_t _count) { maxCount_ = _count; }
    void SetInterval(float _sec) { interval_ = _sec; }

    void Debug();

    bool Empty() const;

    // カメラ調整用の情報取得
    [[nodiscard]] float GetFarthestEnemyDistance(Vector3 referencePos) const;

    [[nodiscard]] uint16_t GetDeathCount() const;

private:
    void Spawn();
    void LoadParams();
    void SaveParams();

}; // class Enemies

#endif // Enemies_HPP_
