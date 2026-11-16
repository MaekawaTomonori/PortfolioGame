#ifndef INPUT_HANDLER_HPP_
#define INPUT_HANDLER_HPP_

#include "Movement/MovementContext.hpp"
#include "Input.hpp"
#include "KeyBinds.hpp"

/**
 * @brief プレイヤー入力をMovementContextに変換
 */
class InputHandler {
    Input* input_ = nullptr;
    KeyBinds binds_;

public:
    /**
     * @brief 初期化
     */
    void Initialize(const KeyBinds& _binds);

    /**
     * @brief 入力を処理してMovementContextを更新
     * @param _context 更新するMovementContext
     * @param _currentPos プレイヤーの現在位置
     */
    void UpdateContext(MovementContext& _context, const Vector3& _currentPos) const;

private:
    Vector3 GetMouseInWorld() const;

    [[nodiscard]] bool IsPress(InputAction _action) const;
    [[nodiscard]] bool IsTrigger(InputAction _action) const;
};

#endif // INPUT_HANDLER_HPP_
