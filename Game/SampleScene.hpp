#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"
#include "Sprite.hpp"
#include "src/Sky/Skybox.hpp"

class SampleScene final : public IScene{
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Model> model_;
    std::unique_ptr<Skybox> sky_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:

}; // class SampleScene

#endif // SampleScene_HPP_
