#ifndef INPUT_HANDLER_HPP_
#define INPUT_HANDLER_HPP_

#include "Movement/MovementContext.hpp"
#include "Input.hpp"
#include "KeyBinds.hpp"
#include "Skill/SkillSlot.hpp"

/**
 * @brief プレイヤー入力から移動とスキルのリクエストを作る
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

    [[nodiscard]] bool IsSkillRequested(SkillSlot _slot) const;

    Vector3 GetMouseInWorld() const;
private:

    [[nodiscard]] bool IsPress(InputAction _action) const;
    [[nodiscard]] bool IsTrigger(InputAction _action) const;
};

#endif // INPUT_HANDLER_HPP_
