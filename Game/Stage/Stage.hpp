#ifndef Stage_HPP_
#define Stage_HPP_
#include <memory>

#include "Model.hpp"
#include "Enemy/Enemies.hpp"
#include "Sky/Skybox.hpp"
#include "Player/Player.hpp"

class Stage {
    ParticleSystem* particle_ = nullptr;

    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Model> terrain_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<Enemies> enemies_;

public:
    void Initialize(ParticleSystem* _particle);
    void Update();
    void Draw() const;
    void Debug() const;

    //Getter
    [[nodiscard]] Player* GetPlayer() const;
}; // class Stage

#endif // Stage_HPP_
