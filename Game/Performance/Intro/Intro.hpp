#ifndef Intro_HPP_
#define Intro_HPP_

class Intro {
    bool cameraDone_ = false;

public:
    void Initialize();
    void Update();

    bool IsFinish()    const { return cameraDone_; }
    bool IsCameraDone() const { return cameraDone_; }
}; // class Intro

#endif // Intro_HPP_
