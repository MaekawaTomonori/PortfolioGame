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
    // refs
    GameObject* owner_ = nullptr;

    // own
    std::vector<std::unique_ptr<Bullet>> bullets_;

    float attackRate_ = 1.f;
    float baseAttackRate_ = 1.f;
    // 最小攻撃間隔
    float minAttackInterval_ = 0.1f;
    Bullet::Params bulletParams_;
    float timer_ = 0.f;

    Vector3 direction_ = {};

    std::optional<std::reference_wrapper<PlayerStatus>> status_;

public:
    /** @brief 発射間隔をリセットし、飛んでいる弾を全て消す（ラウンド開始時） **/
    void Initialize();
    /** @brief 弾の移動と発射間隔の更新。狙う相手の有無に関わらず毎フレーム呼ぶ **/
    void Update();
    void Draw();

    /** @brief 発射間隔が空いていれば、SetDirection の向きへ1発撃つ **/
    void Fire();

    void SetOwner(GameObject* _owner);
    void SetDirection(const Vector3& _direction);

    void SetStatus(PlayerStatus& _status);

    /** @brief 攻撃速度倍率を適用する (as=1.0 で通常速度) **/
    void ApplyAttackSpeed(float _as);

    void LoadParams();
    void SaveParams();

private:
    void Execute();
}; // class Attack

#endif // Attack_HPP_
