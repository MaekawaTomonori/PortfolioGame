#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "GameObject/GameObject.hpp"
#include "Command/ICommand.hpp"
#include <memory>

#include "Collision/Collider.h"

class Enemy : public GameObject {
    struct Status {
        float hp;
        float damage;
    };

    std::unique_ptr<ICommand> moveCommand_;
    GameObject* target_ = nullptr;

    std::unique_ptr<Collision::Collider> collider_;

public:
    // Enemy基本インターフェース
    void Initialize() override;

    // GameObject必須メソッド
    void Update(float deltaTime) override;
    void Draw() override;


    // ターゲット設定
    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }

    // コマンド設定
    void SetMoveCommand(std::unique_ptr<ICommand> command) { moveCommand_ = std::move(command); }

    void OnCollision(const Collision::Collider* _collider);
};

#endif // ENEMY_HPP_