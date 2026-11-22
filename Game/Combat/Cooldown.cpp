#include "Cooldown.hpp"

#include <algorithm>

#include "Time/Time.hpp"

void Cooldown::Start(float _duration) {
    duration_ = std::max(_duration, 0.f);
    remaining_ = duration_;
}

void Cooldown::Update() {
    remaining_ = std::max(remaining_ - Time().GetDeltaTime(), 0.f);
}

void Cooldown::Reset() {
    remaining_ = 0.f;
}

bool Cooldown::IsReady() const {
    return remaining_ <= 0.f;
}

float Cooldown::GetRemaining() const {
    return remaining_;
}

float Cooldown::GetRatio() const {
    return duration_ > 0.f ? remaining_ / duration_ : 0.f;
}
