#ifndef Attack_HPP_
#define Attack_HPP_
#include <memory>
#include <vector>
#include <functional>
#include <optional>

#include "Bullet/Bullet.hpp"
#include "GameObject/GameObject.hpp"
#include "Player/Status/PlayerStatus.hpp"

/// <summary>
/// 攻撃モジュール
/// 自動攻撃を担当する
/// </summary>
class Attack {
    // consts
    const float DeltaTime = 1.f / 60.f;

    // refs
    GameObject* owner_ = nullptr;

    // own
    std::vector<std::unique_ptr<Bullet>> bullets_;

    const float BaseAttackRate = 3.f;
    float timer_ = 0.f;

    Vector3 direction_ = {};

    std::optional<std::reference_wrapper<PlayerStatus>> status_;

public:
    void Initialize();
    void Update();
    void Draw();

    void SetOwner(GameObject* _owner);
    void SetDirection(Vector3 _direction);

    void SetStatus(PlayerStatus& _status);

private:
    void Execute();
}; // class Attack

#endif // Attack_HPP_
