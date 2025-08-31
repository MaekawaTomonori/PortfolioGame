#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "GameObject/GameObject.hpp"
#include "Components/MovementComponent.hpp"
#include "Model.hpp"
#include <memory>

class Player : public GameObject {
private:
    // コンポーネント
    std::unique_ptr<MovementComponent> movement_;
    
    // レンダリング（直接管理）
    std::unique_ptr<Model> model_;

public:
    Player();
    ~Player() override;
    
    void Initialize();
    
    // GameObject override
    void Update(float deltaTime) override;
    void Draw() override;
    
    // Movement設定
    void SetMoveSpeed(float speed);
    const Vector3& GetVelocity() const;
};
#endif // PLAYER_HPP_