#ifndef KeyGuide_HPP_
#define KeyGuide_HPP_
#include <array>
#include <vector>

#include "GEUI.hpp"
#include "Input.hpp"

class KeyGuide {
    /** @brief 押している間だけ色が変わるキー */
    struct MoveKey {
        BYTE code = 0;
        GEUI::Widget box;
        GEUI::Widget label;
    };

    /** @brief クールダウンを持つスキルのキー */
    struct SkillKey {
        BYTE code = 0;
        const char* name = "";
        GEUI::Widget box;
        GEUI::Widget label;
        GEUI::Widget cooldown;
        float cooldownRatio = 0.f;
        float flashTimer = 0.f;
        Vector4 flashColor{};
    };

    GEUI::Canvas ui_;
    GEUI::Widget move_;
    GEUI::Widget skills_;
    std::vector<MoveKey> moveKeys_;
    std::array<SkillKey, 2> skillKeys_;

    Input* input_ = nullptr;

    float fadeElapsed_ = 0.f;

public:
    ~KeyGuide();

    void Initialize();
    void Update();
    void Debug();
    void SetActive(bool _active);

    /** @brief 指定キーのCT比率を設定する (0=ready, 1=full cooldown) **/
    void SetCooldown(BYTE _key, float _ratio);

private:
    static MoveKey FindMoveKey(const GEUI::Widget& _parent, BYTE _code, const char* _name);
    static SkillKey FindSkillKey(const GEUI::Widget& _parent, BYTE _code, const char* _name);
    void UpdateSkillKey(SkillKey& _key, float _deltaTime) const;
    static void Flash(SkillKey& _key, const Vector4& _color);

}; // class KeyGuide

#endif // KeyGuide_HPP_
