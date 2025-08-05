#ifndef MyGame_HPP_
#define MyGame_HPP_
#include "IGame.hpp"

class MyGame final : public IGame{
public:
    MyGame();
    ~MyGame() override;
}; // class MyGame

#endif // MyGame_HPP_
