#include "UiTheme.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "imgui.h"
#include "Json/JsonParams.hpp"
#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "externals/json/json.hpp"

namespace {
    constexpr const char* CANVAS_DIRECTORY = "Assets/UI";

    constexpr const char* FILE_NAME = "UiTheme";
    constexpr const char* BASE_GROUP = "Base";
    constexpr const char* BASE_KEY = "Color";
    constexpr const char* FIXED_GROUP = "Fixed";
    constexpr const char* DERIVED_GROUP = "Derived";

    constexpr const char* COLOR_NAMES[] = {"Panel", "PanelSolid", "Accent", "Highlight", "Text", "TextDim", "Locked", "Danger"};
    static_assert(std::size(COLOR_NAMES) == static_cast<size_t>(UiColor::Count));

    // JSON に無いときの値。Accent と Highlight はテーマ色から作るので使わない
    constexpr Vector4 DEFAULT_COLORS[] = {
        {0.f, 0.f, 0.f, 0.5f},
        {0.f, 0.f, 0.f, 0.9f},
        {},
        {},
        {1.f, 1.f, 1.f, 1.f},
        {0.8f, 0.8f, 0.8f, 1.f},
        {0.35f, 0.35f, 0.35f, 1.f},
        {0.95f, 0.3f, 0.3f, 1.f},
    };
    static_assert(std::size(DEFAULT_COLORS) == static_cast<size_t>(UiColor::Count));

    bool IsDerived(const UiColor _color) {
        return _color == UiColor::Accent || _color == UiColor::Highlight;
    }

    struct Hsv {
        float hue = 0.f;
        float saturation = 0.f;
        float value = 0.f;
    };

    Hsv ToHsv(const Vector4& _color) {
        const float maxValue = std::max({_color.x, _color.y, _color.z});
        const float minValue = std::min({_color.x, _color.y, _color.z});
        const float range = maxValue - minValue;

        Hsv hsv{.saturation = maxValue > 0.f ? range / maxValue : 0.f, .value = maxValue};
        if (range <= 0.f) return hsv;

        if (maxValue == _color.x) {
            hsv.hue = (_color.y - _color.z) / range;
        } else if (maxValue == _color.y) {
            hsv.hue = 2.f + (_color.z - _color.x) / range;
        } else {
            hsv.hue = 4.f + (_color.x - _color.y) / range;
        }
        hsv.hue /= 6.f;
        if (hsv.hue < 0.f) hsv.hue += 1.f;
        return hsv;
    }

    Vector4 ToRgb(const Hsv& _hsv, const float _alpha) {
        const float h = _hsv.hue * 6.f;
        const float chroma = _hsv.value * _hsv.saturation;
        const float x = chroma * (1.f - std::abs(std::fmod(h, 2.f) - 1.f));
        const float m = _hsv.value - chroma;

        float r = 0.f, g = 0.f, b = 0.f;
        switch (static_cast<int>(h) % 6) {
        case 0: r = chroma; g = x; break;
        case 1: r = x; g = chroma; break;
        case 2: g = chroma; b = x; break;
        case 3: g = x; b = chroma; break;
        case 4: r = x; b = chroma; break;
        default: r = chroma; b = x; break;
        }
        return {r + m, g + m, b + m, _alpha};
    }

    std::optional<UiColor> FindColor(const std::string& _name) {
        for (size_t i = 0; i < std::size(COLOR_NAMES); ++i) {
            if (_name == COLOR_NAMES[i]) return static_cast<UiColor>(i);
        }
        return std::nullopt;
    }

    nlohmann::ordered_json ToJson(const Vector4& _color) {
        const auto round = [](const float _value) { return std::round(static_cast<double>(_value) * 1000.0) / 1000.0; };
        return nlohmann::ordered_json::array({round(_color.x), round(_color.y), round(_color.z), round(_color.w)});
    }

    bool ApplyRole(nlohmann::ordered_json& _target, const nlohmann::ordered_json& _role, const UiTheme& _theme) {
        if (!_role.is_string()) return false;

        const auto color = FindColor(_role.get<std::string>());
        if (!color) {
            Log::Send(Log::Level::WARNING, "UiTheme: unknown color role " + _role.get<std::string>());
            return false;
        }
        _target = ToJson(_theme.Get(*color));
        return true;
    }

    bool ApplyRoles(nlohmann::ordered_json& _node, const UiTheme& _theme) {
        bool changed = false;
        if (_node.is_object()) {
            if (_node.contains("colorRole")) {
                changed |= ApplyRole(_node["color"], _node["colorRole"], _theme);
            }
            if (_node.contains("selectedRole") && _node.contains("selectedStyle") && _node["selectedStyle"].is_object()) {
                changed |= ApplyRole(_node["selectedStyle"]["color"], _node["selectedRole"], _theme);
            }
        }
        if (_node.is_structured()) {
            for (auto& child : _node) {
                changed |= ApplyRoles(child, _theme);
            }
        }
        return changed;
    }
}

void UiTheme::Load() {
    for (size_t i = 0; i < colors_.size(); ++i) {
        colors_[i] = DEFAULT_COLORS[i];
    }

    const auto& json = Singleton<JsonParams>::GetInstance();
    if (json->Load(FILE_NAME)) {
        json->TryGetValue(FILE_NAME, BASE_GROUP, BASE_KEY, base_);
        for (size_t i = 0; i < colors_.size(); ++i) {
            if (IsDerived(static_cast<UiColor>(i))) continue;
            json->TryGetValue(FILE_NAME, FIXED_GROUP, COLOR_NAMES[i], colors_[i]);
        }
        json->TryGetValue(FILE_NAME, DERIVED_GROUP, "AccentAlpha", accentAlpha_);
        json->TryGetValue(FILE_NAME, DERIVED_GROUP, "HighlightSaturation", highlightSaturation_);
        json->TryGetValue(FILE_NAME, DERIVED_GROUP, "HighlightValue", highlightValue_);
    }
    Rebuild();
}

void UiTheme::Save() const {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->SetValue(FILE_NAME, BASE_GROUP, BASE_KEY, base_);
    for (size_t i = 0; i < colors_.size(); ++i) {
        if (IsDerived(static_cast<UiColor>(i))) continue;
        json->SetValue(FILE_NAME, FIXED_GROUP, COLOR_NAMES[i], colors_[i]);
    }
    json->SetValue(FILE_NAME, DERIVED_GROUP, "AccentAlpha", accentAlpha_);
    json->SetValue(FILE_NAME, DERIVED_GROUP, "HighlightSaturation", highlightSaturation_);
    json->SetValue(FILE_NAME, DERIVED_GROUP, "HighlightValue", highlightValue_);
    json->Save(FILE_NAME);
}

void UiTheme::Debug() {
#ifdef _DEBUG
    ImGui::Begin("UiTheme");

    ImGui::SeparatorText("Theme");
    bool changed = ImGui::ColorEdit3("Base", &base_.x);
    changed |= ImGui::SliderFloat("Accent Alpha", &accentAlpha_, 0.f, 1.f);
    changed |= ImGui::SliderFloat("Highlight Saturation", &highlightSaturation_, 0.f, 1.f);
    changed |= ImGui::SliderFloat("Highlight Value", &highlightValue_, 0.f, 1.f);
    if (changed) {
        Rebuild();
    }
    for (size_t i = 0; i < colors_.size(); ++i) {
        if (!IsDerived(static_cast<UiColor>(i))) continue;
        const Vector4& color = colors_[i];
        ImGui::ColorButton(COLOR_NAMES[i], {color.x, color.y, color.z, color.w}, ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::SameLine();
        ImGui::Text("%s", COLOR_NAMES[i]);
    }

    ImGui::SeparatorText("Fixed");
    for (size_t i = 0; i < colors_.size(); ++i) {
        if (IsDerived(static_cast<UiColor>(i))) continue;
        ImGui::ColorEdit4(COLOR_NAMES[i], &colors_[i].x, ImGuiColorEditFlags_AlphaPreviewHalf);
    }

    ImGui::Separator();
    if (ImGui::Button("Save")) {
        Save();
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply to UI")) {
        ApplyToCanvases();
    }
    ImGui::End();
#endif
}

void UiTheme::ApplyToCanvases() const {
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(CANVAS_DIRECTORY, error)) {
        if (entry.path().extension() != ".json") continue;

        const std::string path = entry.path().string();
        std::ifstream input(path);
        if (!input.is_open()) {
            Log::Send(Log::Level::ERR, "UiTheme: failed to open " + path);
            continue;
        }
        nlohmann::ordered_json root = nlohmann::ordered_json::parse(input, nullptr, false);
        input.close();
        if (root.is_discarded()) {
            Log::Send(Log::Level::ERR, "UiTheme: invalid canvas json " + path);
            continue;
        }

        if (!ApplyRoles(root, *this)) continue;

        std::ofstream output(path);
        if (!output.is_open()) {
            Log::Send(Log::Level::ERR, "UiTheme: failed to write " + path);
            continue;
        }
        output << root.dump(4) << '\n';
        Log::Send(Log::Level::INFO, "UiTheme: applied " + path);
    }
    if (error) {
        Log::Send(Log::Level::ERR, "UiTheme: failed to read " + std::string(CANVAS_DIRECTORY));
    }
}

const Vector4& UiTheme::Get(const UiColor _color) const {
    return colors_[static_cast<size_t>(_color)];
}

void UiTheme::Rebuild() {
    const Hsv base = ToHsv(base_);

    colors_[static_cast<size_t>(UiColor::Accent)] = {base_.x, base_.y, base_.z, accentAlpha_};
    colors_[static_cast<size_t>(UiColor::Highlight)] = ToRgb({base.hue, base.saturation * highlightSaturation_, highlightValue_}, 1.f);
}
