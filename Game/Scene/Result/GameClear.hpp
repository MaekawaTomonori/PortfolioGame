#ifndef GameClear_HPP_
#define GameClear_HPP_
#include "IScene.hpp"
#include "Sprite.hpp"
#include "Showcase/ItemShowcase.hpp"

class GameClear : public IScene {
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<ItemShowcase> showcase_;

    float time_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

}; // class GameClear

#endif // GameClear_HPP_
