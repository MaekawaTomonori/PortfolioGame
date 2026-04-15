#ifndef Enemies_HPP_
#define Enemies_HPP_
#include <vector>
#include <memory>

#include "Enemy.hpp"
#include "ParticleSystem/ParticleSystem.hpp"
#include "Status/GameStatus.hpp"

class Enemies {
    const float SIZE = 0.5f;
    
    ParticleSystem* particle_ = nullptr;
    const GameStatus& status_;

    GameObject* target_ = nullptr;
    std::vector<std::unique_ptr<Enemy>> enemies_;

    bool done_ = false;

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
    Enemies(ParticleSystem* _particle, const GameStatus& _status);
    void Initialize();
    void Update();
    void Draw() const;

    [[nodiscard]] Vector3 GetNearest(const Vector3& _pos) const;

    void SetTarget(GameObject* _target);

    void Debug();

    bool IsDone() const;
    bool Empty() const;

    // カメラ調整用の情報取得
    [[nodiscard]] float GetFarthestEnemyDistance(const Vector3& referencePos) const;

    [[nodiscard]] uint16_t GetDeathCount() const;

    [[nodiscard]] const std::vector<std::unique_ptr<Enemy>>& GetAll() const { return enemies_; }

private:
    void Spawn();
    void LoadParams();
    void SaveParams();

}; // class Enemies

#endif // Enemies_HPP_
