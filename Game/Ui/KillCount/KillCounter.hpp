#ifndef KillCounter_HPP_
#define KillCounter_HPP_
#include <memory>

#include "Ui/UserInterface.hpp"

class Enemies;

class KillCounter {
    const Vector2 TEXTURE_SIZE = {64.f, 96.f};

    Enemies* enemies_ = nullptr;
    uint16_t max_ = 0;

    std::unique_ptr<Ui::Canvas> ui_;

public:
    void Initialize(Enemies* _enemies, uint16_t _max);
    void Update();
    void Draw() const;
    void Debug();
}; // class KillCounter

#endif // KillCounter_HPP_
