#ifndef KillCounter_HPP_
#define KillCounter_HPP_
#include <cstdint>

#include "GEUI.hpp"

class Enemies;

class KillCounter {
    GEUI::Canvas ui_;
    GEUI::Widget root_;
    GEUI::Widget text_;
    GEUI::Widget fill_;

    Enemies* enemies_ = nullptr;
    uint16_t max_ = 0;
    uint16_t shown_ = 0;

    float fadeElapsed_ = 0.f;

public:
    ~KillCounter();

    void Initialize();
    void Update();
    void Debug();

    /** @brief 数える対象と目標数を設定する（ラウンド開始ごとに呼ぶ） */
    void Setup(Enemies* _enemies, uint16_t _max);
    void SetActive(bool _active);
    void PlayIntroAnim();

private:
    void ApplyCount(uint16_t _count);

}; // class KillCounter

#endif // KillCounter_HPP_
