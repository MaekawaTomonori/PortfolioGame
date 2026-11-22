#ifndef MY_GAME_HPP_
#define MY_GAME_HPP_
#include "IGame.hpp"

class MyGame final : public IGame{
public:
    void Initialize(GameEngine::Config& _config) override;

private:
    void Register();
}; // class MyGame

#endif // MY_GAME_HPP_
