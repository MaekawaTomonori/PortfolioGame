#include "Movement.hpp"
#include "GameObject/GameObject.hpp"
#include <algorithm>

void Movement::Initialize(GameObject* _owner) {
    owner_ = _owner;
}

void Movement::Update(MovementContext& _context) {
    if (!owner_) return;

    _context.owner = owner_;

    // 優先度の高い順に並んでいるので、最初に実行できたものだけを使う
    for (auto* behavior : behaviors_) {
        if (behavior->CanExecute(_context)) {
            Vector3 velocity = behavior->Calculate(_context);
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
    if (!_behavior) return;

    behaviors_.push_back(_behavior);
    // 優先度は途中で変わらないので、足したときに一度だけ並べ直す
    std::ranges::stable_sort(behaviors_,
        [](const IMovementBehavior* _a, const IMovementBehavior* _b) {
            return _a->GetPriority() > _b->GetPriority();
        });
}

void Movement::ClearBehaviors() {
    behaviors_.clear();
}
