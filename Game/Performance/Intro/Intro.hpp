#ifndef Intro_HPP_
#define Intro_HPP_
#include <memory>

#include "Sprite.hpp"


class Intro {
    bool cameraDone_ = false;
    bool spriteDone_ = false;

    std::unique_ptr<Sprite> sprite_;
    float duration_ = 3.f;
    float time_ = 0.f;

public:
    void Initialize();
    void Update();
    void Draw();

    bool IsFinish() const;
    bool IsCameraDone() const { return cameraDone_; }
private:

}; // class Intro

#endif // Intro_HPP_
