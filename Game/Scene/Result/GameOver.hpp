#ifndef GAME_OVER_HPP_
#define GAME_OVER_HPP_
#include <memory>

#include "IScene.hpp"
#include "Sprite.hpp"
#include "Ui/MenuInput.hpp"


class GameOver : public IScene {
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Sprite> bg_;
    MenuInput menuInput_;

    float alpha_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:
    void OnEnable() override;
}; // class GameOver

#endif // GAME_OVER_HPP_
