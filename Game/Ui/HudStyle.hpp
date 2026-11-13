#ifndef HUDSTYLE_HPP_
#define HUDSTYLE_HPP_
#include "GEUI.hpp"

/** @brief HUD 共通の色と小さな表示ヘルパー
 * Assets/UI の HUD 用 JSON（Timer / KillCounter / KeyGuide / StageInfo）と同じ値を使う。
 */
namespace HudStyle {
    constexpr Vector4 WHITE  = {1.f, 1.f, 1.f, 1.f};
    constexpr Vector4 GRAY   = {0.8f, 0.8f, 0.8f, 1.f};
    constexpr Vector4 BOX    = {0.f, 0.f, 0.f, 0.5f};
    constexpr Vector4 GREEN  = {0.3f, 1.f, 0.3f, 1.f};
    constexpr Vector4 YELLOW = {1.f, 0.9f, 0.1f, 1.f};
    constexpr Vector4 RED    = {1.f, 0.25f, 0.25f, 1.f};

    constexpr float FADE_DURATION = 0.5f;

    inline Vector4 Lerp(const Vector4& _a, const Vector4& _b, float _t) {
        return {
            _a.x + (_b.x - _a.x) * _t,
            _a.y + (_b.y - _a.y) * _t,
            _a.z + (_b.z - _a.z) * _t,
            _a.w + (_b.w - _a.w) * _t,
        };
    }

    /** @brief 残り割合に応じて 緑 → 黄 → 赤 と色を変える */
    inline const Vector4& GaugeColor(float _ratio) {
        if (_ratio > 0.5f) return GREEN;
        if (_ratio > 0.25f) return YELLOW;
        return RED;
    }
}

#endif // HUDSTYLE_HPP_
