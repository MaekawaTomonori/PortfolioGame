#ifndef INTRO_HPP_
#define INTRO_HPP_

class Intro {
    bool cameraDone_ = false;

public:
    void Initialize();
    void Update();

    bool IsFinish()    const { return cameraDone_; }
    bool IsCameraDone() const { return cameraDone_; }
}; // class Intro

#endif // INTRO_HPP_
