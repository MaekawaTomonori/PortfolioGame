#include "MenuInput.hpp"

#include "Pattern/Singleton.hpp"

namespace {
    struct DirectionKey {
        BYTE key;
        GEUI::Direction direction;
    };

    constexpr DirectionKey DIRECTION_KEYS[] = {
        {DIK_UP, GEUI::Direction::Up},       {DIK_W, GEUI::Direction::Up},
        {DIK_DOWN, GEUI::Direction::Down},   {DIK_S, GEUI::Direction::Down},
        {DIK_LEFT, GEUI::Direction::Left},   {DIK_A, GEUI::Direction::Left},
        {DIK_RIGHT, GEUI::Direction::Right}, {DIK_D, GEUI::Direction::Right},
    };
}

MenuInput::MenuInput() {
    input_ = Singleton<Input>::GetInstance();
}

std::optional<GEUI::Direction> MenuInput::GetDirection() const {
    if (!input_) return std::nullopt;

    for (const DirectionKey& key : DIRECTION_KEYS) {
        if (input_->IsTrigger(key.key)) return key.direction;
    }
    return std::nullopt;
}

bool MenuInput::IsSubmit() const {
    return input_ && (input_->IsTrigger(DIK_RETURN) || input_->IsTrigger(DIK_SPACE));
}

bool MenuInput::IsCancel() const {
    return input_ && input_->IsTrigger(DIK_ESCAPE);
}
