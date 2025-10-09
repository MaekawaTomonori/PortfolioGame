#ifndef Stage_HPP_
#define Stage_HPP_
#include <memory>

#include "Model.hpp"
#include "Sky/Skybox.hpp"
#include "Player/Player.hpp"

class Stage {
    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Model> terrain_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<Model> boss_;

public:
    void Initialize();
    void Update();
    void Draw() const;
}; // class Stage

#endif // Stage_HPP_
