#include "HudLayout.hpp"

#include <fstream>
#include <unordered_map>

#include "imgui.h"
#include "Log.hpp"
#include "externals/json/json.hpp"

namespace {
    // HUD の Canvas はすべてこの基準解像度で作っている
    constexpr Vector2 REFERENCE_SIZE = {1280.f, 720.f};

    // 直近の保存結果を Canvas ごとに覚えて、ボタンの横に出す
    std::unordered_map<std::string, bool> lastSaveResults;
}

void HudLayout::DrawEditor(Group& _group) {
    ImGui::PushID(_group.name);
    ImGui::SeparatorText(_group.name);

    Vector2 position = _group.widget.GetOffset() * REFERENCE_SIZE;
    if (ImGui::DragFloat2("Position (px)", &position.x, 1.f)) {
        _group.widget.SetOffset(position / REFERENCE_SIZE);
    }

    float scale = _group.widget.GetScale();
    if (ImGui::SliderFloat("Scale", &scale, 0.5f, 2.f)) {
        _group.widget.SetScale(scale);
    }

    ImGui::PopID();
}

bool HudLayout::Save(const std::string& _canvasPath, std::span<const Group> _groups) {
    std::ifstream input(_canvasPath);
    if (!input.is_open()) {
        Log::Send(Log::Level::ERR, "HudLayout: failed to open " + _canvasPath);
        return false;
    }
    // 例外を使わずに読み、壊れていれば書き戻さない
    nlohmann::json root = nlohmann::json::parse(input, nullptr, false);
    input.close();
    if (root.is_discarded() || !root.contains("children") || !root["children"].is_array()) {
        Log::Send(Log::Level::ERR, "HudLayout: invalid canvas json " + _canvasPath);
        return false;
    }

    for (const Group& group : _groups) {
        bool found = false;
        for (nlohmann::json& item : root["children"]) {
            if (!item.is_object() || item.value("name", "") != group.name) continue;

            const Vector2 offset = group.widget.GetOffset();
            item["offset"] = nlohmann::json::array({offset.x, offset.y});
            item["scale"] = group.widget.GetScale();
            found = true;
            break;
        }
        if (!found) {
            Log::Send(Log::Level::WARNING, std::string("HudLayout: widget not found - ") + group.name + " in " + _canvasPath);
        }
    }

    std::ofstream output(_canvasPath);
    if (!output.is_open()) {
        Log::Send(Log::Level::ERR, "HudLayout: failed to write " + _canvasPath);
        return false;
    }
    output << root.dump(4) << '\n';
    Log::Send(Log::Level::INFO, "HudLayout: saved " + _canvasPath);
    return true;
}

void HudLayout::DrawSaveButton(const std::string& _canvasPath, std::span<const Group> _groups) {
    ImGui::PushID(_canvasPath.c_str());
    if (ImGui::Button("Save Layout")) {
        lastSaveResults[_canvasPath] = Save(_canvasPath, _groups);
    }
    ImGui::SameLine();
    if (const auto it = lastSaveResults.find(_canvasPath); it != lastSaveResults.end()) {
        ImGui::TextDisabled("%s %s", it->second ? "Saved:" : "Failed:", _canvasPath.c_str());
    } else {
        ImGui::TextDisabled("%s", _canvasPath.c_str());
    }
    ImGui::PopID();
}
