#ifndef UI_THEME_HPP_
#define UI_THEME_HPP_

#include <array>

#include "Math/Vector4.hpp"

enum class UiColor {
    Panel,
    PanelSolid,
    Accent,
    Highlight,
    Text,
    TextDim,
    Locked,
    Danger,
    Count,
};

class UiTheme {
    Vector4 base_ = {0.f, 0.7f, 1.f, 1.f};
    float accentAlpha_ = 0.85f;
    float highlightSaturation_ = 0.35f;
    float highlightValue_ = 1.f;

    std::array<Vector4, static_cast<size_t>(UiColor::Count)> colors_{};

public:
    void Load();
    void Save() const;
    void Debug();
    void ApplyToCanvases() const;

    const Vector4& Get(UiColor _color) const;

private:
    void Rebuild();
};

#endif // UI_THEME_HPP_
