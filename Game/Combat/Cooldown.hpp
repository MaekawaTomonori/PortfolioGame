#ifndef COOLDOWN_HPP_
#define COOLDOWN_HPP_

class Cooldown {
    float remaining_ = 0.f;
    float duration_ = 0.f;

public:
    void Start(float _duration);
    void Update();
    void Reset();

    bool IsReady() const;
    
    float GetRemaining() const;

    /** @brief 0 で使える、1 で始まったばかり */
    float GetRatio() const;
};

#endif // COOLDOWN_HPP_
