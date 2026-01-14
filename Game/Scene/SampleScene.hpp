#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"
#include "Sprite.hpp"
#include "Ui/KeyGuide/KeyGuide.hpp"

class SampleScene final : public IScene{
    bool smoke_ = false;
    std::unique_ptr<KeyGuide> keyGuide_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
private:

}; // class SampleScene

#endif // SampleScene_HPP_
