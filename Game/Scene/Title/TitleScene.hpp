#ifndef TitleScene_HPP_
#define TitleScene_HPP_

#include <memory>

#include "IScene.hpp"
#include "Sprite.hpp"
#include "Stage/Stage.hpp"

class TitleScene : public IScene{
    GameStatus status_{};
    std::unique_ptr<Stage> stage_;
    std::unique_ptr<Sprite> titleLogo_;
    std::unique_ptr<Sprite> pushtoStart_;

    float alpha_ = 0.f;
    bool increase_ = true;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
    void Finalize() override;

private:
    void OnEnable() override;
}; // class TitleScene

#endif // TitleScene_HPP_
