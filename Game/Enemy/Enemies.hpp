#ifndef Enemies_HPP_
#define Enemies_HPP_
#include <vector>

#include "Enemy.hpp"
#include "ParticleSystem/ParticleSystem.hpp"

class Enemies {
    ParticleSystem* particle_ = nullptr;

    GameObject* target_ = nullptr;
    std::vector<std::unique_ptr<Enemy>> enemies_;

    const uint16_t MaxEnemies = 3;
    const float Interval = 2.f;
    float timer_ = 0.f;

#ifdef _DEBUG
    bool autoSpawn_ = true;
#endif

public:
    void Initialize(ParticleSystem* _particle);
    void Update();
    void Draw() const;

    [[nodiscard]] Vector3 GetNearest(Vector3 _pos) const;

    void SetTarget(GameObject* _target);

    void Debug();

private:
    void Spawn();

}; // class Enemies

#endif // Enemies_HPP_
