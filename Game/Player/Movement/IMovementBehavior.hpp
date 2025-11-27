#ifndef I_MOVEMENT_BEHAVIOR_HPP_
#define I_MOVEMENT_BEHAVIOR_HPP_

#include "Math/Vector3.hpp"

class Input;

/// <summary>
/// 移動動作の基底インターフェース
/// 各種移動パターン（歩行、ダッシュ、ジャンプなど）を実装するための抽象クラス
/// </summary>
class IMovementBehavior {
public:
    virtual ~IMovementBehavior() = default;

    /// <summary>
    /// 移動ベクトルを計算
    /// </summary>
    /// <param name="input">入力システム</param>
    /// <param name="deltaTime">フレーム時間</param>
    /// <returns>計算された移動ベクトル</returns>
    virtual Vector3 Calculate(const Input* input, float deltaTime) = 0;

    /// <summary>
    /// この動作が実行可能かチェック
    /// </summary>
    /// <param name="input">入力システム</param>
    /// <returns>実行可能な場合true</returns>
    virtual bool CanExecute(const Input* input) const = 0;

    /// <summary>
    /// この動作の優先度を取得（高いほど優先）
    /// </summary>
    /// <returns>優先度</returns>
    virtual int GetPriority() const = 0;
};

#endif // I_MOVEMENT_BEHAVIOR_HPP_
