#ifndef MOVEMENT_HPP_
#define MOVEMENT_HPP_

#include "IMovementBehavior.hpp"
#include <memory>
#include <vector>

class GameObject;

/**
 * @brief 移動動作のハンドラークラス
 */
class Movement {
    std::vector<IMovementBehavior*> behaviors_;
    GameObject* owner_ = nullptr;

public:
    Movement() = default;
    ~Movement() = default;

    /**
     * @brief 初期化
     * @param _owner 所有者のGameObject
     */
    void Initialize(GameObject* _owner);

    /**
     * @brief 更新処理
     * @param context 移動コンテキスト
     * @param _deltaTime フレーム時間
     */
    void Update(MovementContext& context, float _deltaTime);

    void Debug() const;

    /**
     * @brief 移動動作を追加（参照として保持、所有しない）
     * @param _behavior 追加する動作
     */
    void AddBehavior(IMovementBehavior* _behavior);

    /**
     * @brief 全ての動作をクリア
     */
    void ClearBehaviors();

    /**
     * @brief 登録されている動作の数を取得
     * @return 動作の数
     */
    size_t GetBehaviorCount() const { return behaviors_.size(); }
};

#endif // MOVEMENT_HPP_
