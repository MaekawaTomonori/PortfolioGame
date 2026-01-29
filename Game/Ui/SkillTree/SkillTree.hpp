#ifndef SkillTree_HPP_
#define SkillTree_HPP_
#include <memory>

#include "Ui/UserInterface.hpp"

class SkillTree {
    std::unique_ptr<Ui::UserInterface> ui_;



public:
    void Initialize();
    void Update();
    void Draw();
    void Debug();


private:

}; // class SkillTree

#endif // SkillTree_HPP_
