#include "KeyBinds.hpp"

#include "Json/JsonParams.hpp"
#include "Log.hpp"
#include "Pattern/Singleton.hpp"

namespace {
    constexpr const char* FILE_NAME = "KeyBinds";
    constexpr const char* GROUP = "Player";

    struct DefaultBinding {
        InputAction action;
        const char* name;
        const char* key;
    };

    constexpr DefaultBinding DEFAULT_BINDINGS[] = {
        {InputAction::MoveForward, "MoveForward", "W"},
        {InputAction::MoveBack,    "MoveBack",    "S"},
        {InputAction::MoveLeft,    "MoveLeft",    "A"},
        {InputAction::MoveRight,   "MoveRight",   "D"},
        {InputAction::Dodge,       "Dodge",       "SPACE"},
        {InputAction::Ability1,    "Ability1",    "Q"},
        {InputAction::Ability2,    "Ability2",    "F"},
    };
    static_assert(std::size(DEFAULT_BINDINGS) == static_cast<size_t>(InputAction::Count));

    struct KeyName {
        const char* name;
        BYTE code;
    };

    constexpr KeyName KEY_NAMES[] = {
        {"A", DIK_A}, {"B", DIK_B}, {"C", DIK_C}, {"D", DIK_D}, {"E", DIK_E}, {"F", DIK_F},
        {"G", DIK_G}, {"H", DIK_H}, {"I", DIK_I}, {"J", DIK_J}, {"K", DIK_K}, {"L", DIK_L},
        {"M", DIK_M}, {"N", DIK_N}, {"O", DIK_O}, {"P", DIK_P}, {"Q", DIK_Q}, {"R", DIK_R},
        {"S", DIK_S}, {"T", DIK_T}, {"U", DIK_U}, {"V", DIK_V}, {"W", DIK_W}, {"X", DIK_X},
        {"Y", DIK_Y}, {"Z", DIK_Z},
        {"SPACE", DIK_SPACE}, {"LSHIFT", DIK_LSHIFT}, {"LCTRL", DIK_LCONTROL}, {"TAB", DIK_TAB},
        {"ENTER", DIK_RETURN}, {"ESC", DIK_ESCAPE},
        {"UP", DIK_UP}, {"DOWN", DIK_DOWN}, {"LEFT", DIK_LEFT}, {"RIGHT", DIK_RIGHT},
    };

    BYTE ToKeyCode(const std::string& _name) {
        for (const KeyName& key : KEY_NAMES) {
            if (_name == key.name) return key.code;
        }
        return 0;
    }
}

void KeyBinds::Load() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->Load(FILE_NAME);
    const auto groups = json->GetGroups(FILE_NAME);
    const auto group = groups.find(GROUP);

    for (const DefaultBinding& binding : DEFAULT_BINDINGS) {
        std::string label = binding.key;
        if (group != groups.end()) {
            const auto value = group->second.find(binding.name);
            if (value != group->second.end()) {
                if (const auto* name = std::get_if<std::string>(&value->second)) label = *name;
            }
        }

        BYTE key = ToKeyCode(label);
        if (key == 0) {
            Log::Send(Log::Level::WARNING, "KeyBinds: unknown key " + label + " for " + binding.name);
            label = binding.key;
            key = ToKeyCode(label);
        }
        bindings_[static_cast<size_t>(binding.action)] = {key, label};
    }
}

BYTE KeyBinds::GetKey(const InputAction _action) const {
    return Get(_action).key;
}

const std::string& KeyBinds::GetLabel(const InputAction _action) const {
    return Get(_action).label;
}

const KeyBinds::Binding& KeyBinds::Get(const InputAction _action) const {
    return bindings_[static_cast<size_t>(_action)];
}
