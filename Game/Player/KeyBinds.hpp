#ifndef KEY_BINDS_HPP_
#define KEY_BINDS_HPP_

#include <array>
#include <string>

#include "Input.hpp"

enum class InputAction {
    MoveForward,
    MoveBack,
    MoveLeft,
    MoveRight,
    Dodge,
    Ability1,
    Ability2,
    Count,
};

class KeyBinds {
    struct Binding {
        BYTE key = 0;
        std::string label;
    };

    std::array<Binding, static_cast<size_t>(InputAction::Count)> bindings_;

public:
    void Load();

    BYTE GetKey(InputAction _action) const;
    const std::string& GetLabel(InputAction _action) const;

private:
    const Binding& Get(InputAction _action) const;
};

#endif // KEY_BINDINGS_HPP_
