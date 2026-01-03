#include "Movement.hpp"
#include "GameObject/GameObject.hpp"
#include <algorithm>

void Movement::Initialize(GameObject* _owner) {
    owner_ = _owner;
}

void Movement::Update(MovementContext& _context, const float _deltaTime) {
    if (!owner_) return;

    _context.owner = owner_;

    // 優先度順にソート（降順：高い優先度が先）
    std::ranges::sort(behaviors_,
        [](IMovementBehavior* a, IMovementBehavior* b) {
            return a->GetPriority() > b->GetPriority();
        });

    // 実行可能な動作を探して実行
    for (auto* behavior : behaviors_) {
        if (behavior->CanExecute(_context)) {
            Vector3 velocity = behavior->Calculate(_context, _deltaTime);
            owner_->SetVelocity(velocity);
            return;
        }
    }

    // どの動作も実行できない場合は速度をゼロに
    owner_->SetVelocity({});
}

void Movement::Debug() const {
    for (const auto& behavior : behaviors_) {
        behavior->Debug();
    }
}

void Movement::AddBehavior(IMovementBehavior* _behavior) {
    if (_behavior) {
        behaviors_.push_back(_behavior);
    }
}

void Movement::ClearBehaviors() {
    behaviors_.clear();
}
