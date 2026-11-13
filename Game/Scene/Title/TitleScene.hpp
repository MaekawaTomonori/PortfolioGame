#ifndef TitleScene_HPP_
#define TitleScene_HPP_

#include <memory>

#include "GEUI.hpp"
#include "IScene.hpp"
#include "Stage/Stage.hpp"

class TitleScene : public IScene{
    GameStatus status_{};
    std::unique_ptr<Stage> stage_;

    GEUI::Canvas ui_;
    GEUI::Widget start_;
    GEUI::Widget quit_;

public:
    ~TitleScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
    void Finalize() override;

private:
    void OnEnable() override;
    void HandleInput();
}; // class TitleScene

#endif // TitleScene_HPP_
