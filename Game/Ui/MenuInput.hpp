#ifndef MENU_INPUT_HPP_
#define MENU_INPUT_HPP_

#include <optional>

#include "GEUI.hpp"
#include "Input.hpp"

class MenuInput {
    Input* input_ = nullptr;

public:
    MenuInput();

    [[nodiscard]] std::optional<GEUI::Direction> GetDirection() const;
    [[nodiscard]] bool IsSubmit() const;
    [[nodiscard]] bool IsCancel() const;
};

#endif // MENU_INPUT_HPP_
