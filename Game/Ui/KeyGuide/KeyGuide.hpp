#ifndef KeyGuide_HPP_
#define KeyGuide_HPP_
#include <array>
#include <vector>

#include "GEUI.hpp"
#include "Input.hpp"
#include "Player/KeyBinds.hpp"

class UiTheme;

class KeyGuide {
    /** @brief 押している間だけ色が変わるキー */
    struct MoveKey {
        InputAction action{};
        GEUI::Widget box;
        GEUI::Widget label;
    };

    /** @brief クールダウンを持つスキルのキー */
    struct SkillKey {
        InputAction action{};
        GEUI::Widget box;
        GEUI::Widget label;
        GEUI::Widget cooldown;
        float cooldownRatio = 0.f;
        float flashTimer = 0.f;
        Vector4 flashColor{};
    };

    KeyBinds binds_;
    const UiTheme* theme_ = nullptr;

    GEUI::Canvas ui_;
    GEUI::Widget move_;
    GEUI::Widget skills_;
    std::vector<MoveKey> moveKeys_;
    std::array<SkillKey, 2> skillKeys_;

    Input* input_ = nullptr;

    float fadeElapsed_ = 0.f;

public:
    ~KeyGuide();

    void Initialize(const KeyBinds& _binds, const UiTheme& _theme);
    void Update();
    void Debug();
    void SetActive(bool _active);

    /** @brief 指定キーのCT比率を設定する (0=ready, 1=full cooldown) **/
    void SetCooldown(InputAction _action, float _ratio);

private:
    static MoveKey FindMoveKey(const GEUI::Widget& _parent, InputAction _action, const char* _name);
    static SkillKey FindSkillKey(const GEUI::Widget& _parent, InputAction _action, const char* _name);
    void UpdateSkillKey(SkillKey& _key, float _deltaTime) const;
    static void Flash(SkillKey& _key, const Vector4& _color);

}; // class KeyGuide

#endif // KeyGuide_HPP_
