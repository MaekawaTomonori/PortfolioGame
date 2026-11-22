#ifndef GAME_CLEAR_HPP_
#define GAME_CLEAR_HPP_
#include "IScene.hpp"
#include "Sprite.hpp"
#include "Showcase/ItemShowcase.hpp"
#include "Ui/MenuInput.hpp"

class GameClear : public IScene {
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<ItemShowcase> showcase_;
    MenuInput menuInput_;

    float time_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;

private:
    void OnEnable() override;
}; // class GameClear

#endif // GAME_CLEAR_HPP_
