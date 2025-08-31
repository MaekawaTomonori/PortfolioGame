#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "GameObject/GameObject.hpp"
#include "Components/MovementComponent.hpp"
#include "Model.hpp"
#include <memory>

class MovementComponent;

class Player : public GameObject {
private:
    std::unique_ptr<MovementComponent> movement_;
    
    std::unique_ptr<Model> model_;

public:
    Player();
    ~Player() override;
    
    void Initialize();
    
    void Update(float deltaTime) override;
    void Draw() override;
    
    void SetMoveSpeed(float speed);
    const Vector3& GetVelocity() const;
};
#endif // PLAYER_HPP_