#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "GameObject/GameObject.hpp"
#include "Components/BehaviorComponent.hpp"
#include "Model.hpp"
#include <memory>

class Enemy : public GameObject {
private:
    std::unique_ptr<BehaviorComponent> behavior_;
    std::unique_ptr<Model> model_;
    
public:
    Enemy();
    virtual ~Enemy();
    
    // GameObject必須メソッド
    void Update(float deltaTime) override;
    void Draw() override;
    
    // Enemy基本インターフェース  
    virtual void Initialize();
    
    // ターゲット設定
    void SetTarget(GameObject* target);
    GameObject* GetTarget() const;
};

#endif // ENEMY_HPP_