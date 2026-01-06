#ifndef GameOver_HPP_
#define GameOver_HPP_
#include <memory>

#include "IScene.hpp"
#include "Sprite.hpp"


class GameOver : public IScene {
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Sprite> bg_;

    float alpha_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

}; // class GameOver

#endif // GameOver_HPP_
