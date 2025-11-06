#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "GameObject/GameObject.hpp"
#include "Command/ICommand.hpp"
#include "Model.hpp"
#include <memory>

class Enemy : public GameObject {
private:
    std::unique_ptr<ICommand> moveCommand_;
    GameObject* target_;
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
    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }

    // コマンド設定
    void SetMoveCommand(std::unique_ptr<ICommand> command) { moveCommand_ = std::move(command); }
};

#endif // ENEMY_HPP_