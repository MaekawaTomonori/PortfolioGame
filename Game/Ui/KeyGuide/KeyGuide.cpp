#define NOMINMAX
#include "KeyGuide.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <string>

#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "Ui/HudLayout.hpp"
#include "Ui/HudStyle.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr const char* CANVAS_PATH = "Assets/UI/KeyGuide.json";

    constexpr float FLASH_DURATION = 0.3f;

    // 押している間は黄色くする
    constexpr Vector4 PRESSED_BOX   = {1.f, 0.9f, 0.1f, 0.9f};
    constexpr Vector4 PRESSED_LABEL = {0.f, 0.f, 0.f, 1.f};

    constexpr float COOLDOWN_EPSILON = 0.001f;
}

KeyGuide::~KeyGuide() {
    ui_.Unload();
}

void KeyGuide::Initialize(const KeyBinds& _binds) {
    input_ = Singleton<Input>::GetInstance();
    binds_ = _binds;

    ui_ = GEUI::Canvas::Load(CANVAS_PATH);
    ui_.SetVisible(false);

    move_ = ui_.Find("Move");
    moveKeys_ = {
        FindMoveKey(move_, InputAction::MoveForward, "MoveForward"),
        FindMoveKey(move_, InputAction::MoveLeft, "MoveLeft"),
        FindMoveKey(move_, InputAction::MoveBack, "MoveBack"),
        FindMoveKey(move_, InputAction::MoveRight, "MoveRight"),
        FindMoveKey(move_, InputAction::Dodge, "Dodge"),
    };
    for (MoveKey& key : moveKeys_) {
        key.label.SetText(binds_.GetLabel(key.action));
    }

    skills_ = ui_.Find("Skills");
    skillKeys_ = {
        FindSkillKey(skills_, InputAction::Ability1, "Ability1"),
        FindSkillKey(skills_, InputAction::Ability2, "Ability2"),
    };
    for (SkillKey& key : skillKeys_) {
        SetCooldown(key.action, 0.f);
    }
}

void KeyGuide::Update() {
    if (!ui_.IsVisible()) return;

    const float deltaTime = Time().GetDeltaTime();

    fadeElapsed_ = std::min(fadeElapsed_ + deltaTime, HudStyle::FADE_DURATION);
    ui_.SetAlpha(fadeElapsed_ / HudStyle::FADE_DURATION);

    for (MoveKey& key : moveKeys_) {
        const bool pressed = input_->IsPress(binds_.GetKey(key.action));
        key.box.SetColor(pressed ? PRESSED_BOX : HudStyle::BOX);
        key.label.SetColor(pressed ? PRESSED_LABEL : HudStyle::WHITE);
    }

    for (SkillKey& key : skillKeys_) {
        if (key.cooldownRatio > COOLDOWN_EPSILON && input_->IsTrigger(binds_.GetKey(key.action))) {
            Flash(key, HudStyle::RED);
        }
        UpdateSkillKey(key, deltaTime);
    }
}

void KeyGuide::Debug() {
#ifdef _DEBUG
    ImGui::Begin("KeyGuide");
    for (SkillKey& key : skillKeys_) {
        const std::string label = "Cooldown##" + std::to_string(static_cast<int>(key.action));
        float ratio = key.cooldownRatio;
        if (ImGui::SliderFloat(label.c_str(), &ratio, 0.f, 1.f)) {
            SetCooldown(key.action, ratio);
        }
    }

    HudLayout::Group groups[] = {{"Move", move_}, {"Skills", skills_}};
    for (HudLayout::Group& group : groups) {
        HudLayout::DrawEditor(group);
    }
    HudLayout::DrawSaveButton(CANVAS_PATH, groups);
    ImGui::End();
#endif
}

void KeyGuide::SetActive(const bool _active) {
    if (_active && !ui_.IsVisible()) {
        fadeElapsed_ = 0.f;
        ui_.SetAlpha(0.f);
    }
    ui_.SetVisible(_active);
}

void KeyGuide::SetCooldown(const InputAction _action, const float _ratio) {
    for (SkillKey& key : skillKeys_) {
        if (key.action != _action) continue;

        const float ratio = std::clamp(_ratio, 0.f, 1.f);
        const bool ready = ratio <= COOLDOWN_EPSILON;
        // 使えるようになったら緑に光らせる
        if (key.cooldownRatio > COOLDOWN_EPSILON && ready) {
            Flash(key, HudStyle::GREEN);
        }
        key.cooldownRatio = ratio;

        // CT中は残りを % で出す
        key.cooldown.SetSize({1.f, ratio});
        key.label.SetText(ready ? binds_.GetLabel(key.action) : std::format("{}%", static_cast<int>(std::ceil(ratio * 100.f))));
        return;
    }
}

KeyGuide::MoveKey KeyGuide::FindMoveKey(const GEUI::Widget& _parent, const InputAction _action, const char* _name) {
    const GEUI::Widget box = _parent.Find(_name);
    return {.action = _action, .box = box, .label = box.Find("Label")};
}

KeyGuide::SkillKey KeyGuide::FindSkillKey(const GEUI::Widget& _parent, const InputAction _action, const char* _name) {
    const GEUI::Widget box = _parent.Find(_name);
    return {
        .action = _action,
        .box = box,
        .label = box.Find("Label"),
        .cooldown = box.Find("Cooldown"),
    };
}

void KeyGuide::UpdateSkillKey(SkillKey& _key, const float _deltaTime) const {
    const bool ready = _key.cooldownRatio <= COOLDOWN_EPSILON;
    const bool pressed = ready && input_->IsPress(binds_.GetKey(_key.action));

    _key.label.SetColor(pressed ? PRESSED_LABEL : (ready ? HudStyle::WHITE : HudStyle::GRAY));

    _key.flashTimer = std::max(0.f, _key.flashTimer - _deltaTime);
    const float t = _key.flashTimer / FLASH_DURATION;
    const Vector4& box = pressed ? PRESSED_BOX : HudStyle::BOX;
    _key.box.SetColor(HudStyle::Lerp(box, _key.flashColor, t));
}

void KeyGuide::Flash(SkillKey& _key, const Vector4& _color) {
    _key.flashTimer = FLASH_DURATION;
    _key.flashColor = _color;
}
