#ifndef TitleScene_HPP_
#define TitleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"

class TitleScene : public IScene{
    std::unique_ptr<Model> model_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
}; // class TitleScene

#endif // TitleScene_HPP_
