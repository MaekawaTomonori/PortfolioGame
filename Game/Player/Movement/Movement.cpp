#include "Movement.hpp"
#include "GameObject/GameObject.hpp"
#include "Input.hpp"
#include "Pattern/Singleton.hpp"
#include <algorithm>

void Movement::Initialize(GameObject* _owner) {
    owner_ = _owner;
    input_ = Singleton<Input>::GetInstance();
}

void Movement::Update(const float _deltaTime) {
    if (!owner_ || !input_) return;

    // 優先度順にソート（降順：高い優先度が先）
    std::ranges::sort(behaviors_,
        [](const std::unique_ptr<IMovementBehavior>& a, const std::unique_ptr<IMovementBehavior>& b) {
            return a->GetPriority() > b->GetPriority();
        });

    // 実行可能な動作を探して実行
    for (const auto& behavior : behaviors_) {
        if (behavior->CanExecute(input_)) {
            Vector3 velocity = behavior->Calculate(input_, _deltaTime);
            owner_->SetVelocity(velocity);
            return; // 最初に実行可能な動作のみを適用
        }
    }

    // どの動作も実行できない場合は速度をゼロに
    owner_->SetVelocity({});
}

void Movement::AddBehavior(std::unique_ptr<IMovementBehavior> _behavior) {
    if (_behavior) {
        behaviors_.push_back(std::move(_behavior));
    }
}

void Movement::ClearBehaviors() {
    behaviors_.clear();
}
