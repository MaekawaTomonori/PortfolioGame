#ifndef HUD_STYLE_HPP_
#define HUD_STYLE_HPP_
#include "GEUI.hpp"

/** @brief HUD 共通の値と小さな表示ヘルパー（色は UiTheme から取る） */
namespace HudStyle {
    // HUD の Canvas はすべてこの基準解像度で作っている
    constexpr Vector2 REFERENCE_SIZE = {1280.f, 720.f};

    constexpr float FADE_DURATION = 0.5f;

    inline Vector4 Lerp(const Vector4& _a, const Vector4& _b, float _t) {
        return {
            _a.x + (_b.x - _a.x) * _t,
            _a.y + (_b.y - _a.y) * _t,
            _a.z + (_b.z - _a.z) * _t,
            _a.w + (_b.w - _a.w) * _t,
        };
    }
}

#endif // HUD_STYLE_HPP_
