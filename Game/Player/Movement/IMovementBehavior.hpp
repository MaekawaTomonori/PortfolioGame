#ifndef I_MOVEMENT_BEHAVIOR_HPP_
#define I_MOVEMENT_BEHAVIOR_HPP_

#include "Math/Vector3.hpp"
#include "MovementContext.hpp"

/**
 * @brief 移動動作の基底インターフェース
 */
class IMovementBehavior {
public:
    virtual ~IMovementBehavior() = default;

    /**
     * @brief 移動ベクトルを計算
     * @param context 移動コンテキスト
     * @param deltaTime フレーム時間
     * @return 計算された移動ベクトル
     */
    virtual Vector3 Calculate(const MovementContext& context, float deltaTime) = 0;

    /**
     * @brief この動作が実行可能かチェック
     * @param context 移動コンテキスト
     * @return 実行可能な場合true
     */
    virtual bool CanExecute(const MovementContext& context) = 0;

    /**
     * @brief この動作の優先度を取得（高いほど優先）
     * @return 優先度
     */
    virtual int GetPriority() const = 0;

    virtual void Debug() const {}
};

#endif // I_MOVEMENT_BEHAVIOR_HPP_
