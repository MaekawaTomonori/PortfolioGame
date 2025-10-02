#ifndef TitleScene_HPP_
#define TitleScene_HPP_

#include <memory>

#include "IScene.hpp"
#include "Stage/Stage.hpp"

class TitleScene : public IScene{
    std::unique_ptr<Stage> stage_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
}; // class TitleScene

#endif // TitleScene_HPP_
