#ifndef MOVEMENT_HPP_
#define MOVEMENT_HPP_

#include "IMovementBehavior.hpp"
#include <memory>
#include <vector>


class GameObject;

/// <summary>
/// 移動動作のハンドラークラス
/// 複数の移動動作を管理し、優先度に基づいて適切な動作を選択・実行する
/// </summary>
class Movement {
    std::vector<std::unique_ptr<IMovementBehavior>> behaviors_;
    Input* input_ = nullptr;
    GameObject* owner_ = nullptr;

public:
    Movement() = default;
    ~Movement() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="_owner">所有者のGameObject</param>
    void Initialize(GameObject* _owner);

    /// <summary>
    /// 更新処理
    /// 優先度の高い順に動作をチェックし、実行可能な動作を適用
    /// </summary>
    /// <param name="_deltaTime">フレーム時間</param>
    void Update(float _deltaTime);

    /// <summary>
    /// 移動動作を追加
    /// </summary>
    /// <param name="_behavior">追加する動作</param>
    void AddBehavior(std::unique_ptr<IMovementBehavior> _behavior);

    /// <summary>
    /// 全ての動作をクリア
    /// </summary>
    void ClearBehaviors();

    /// <summary>
    /// 登録されている動作の数を取得
    /// </summary>
    size_t GetBehaviorCount() const { return behaviors_.size(); }
};

#endif // MOVEMENT_HPP_
