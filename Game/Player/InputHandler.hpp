#ifndef INPUT_HANDLER_HPP_
#define INPUT_HANDLER_HPP_

#include "Movement/MovementContext.hpp"
#include "Input.hpp"
#include "Scene/Result/GameClear.hpp"

/**
 * @brief プレイヤー入力をMovementContextに変換
 */
class InputHandler {
    Input* input_ = nullptr;

    // ViewPort
    // TODO: 固定としているが、全画面に対応するべき
    const Vector2 Viewport = {1280.f, 720.f};

public:
    /**
     * @brief 初期化
     */
    void Initialize();

    /**
     * @brief 入力を処理してMovementContextを更新
     * @param context 更新するMovementContext
     * @param currentPos プレイヤーの現在位置
     */
    void UpdateContext(MovementContext& context, const Vector3& currentPos);

private:
    Vector3 GetMouseInWorld() const;
};

#endif // INPUT_HANDLER_HPP_
