#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"
#include "Sprite.hpp"
#include "Ui/UserInterface.hpp"
#include "Ui/SkillTree/SkillTree.hpp"

class SampleScene final : public IScene{
    bool smoke_ = false;
    std::unique_ptr<Ui::UserInterface> userInterface_;
    std::unique_ptr<Model> model_;

    std::unique_ptr<GameStatus> status_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
private:

}; // class SampleScene

#endif // SampleScene_HPP_
