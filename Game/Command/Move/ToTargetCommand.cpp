#include "ToTargetCommand.hpp"
#include "GameObject/GameObject.hpp"

ToTargetCommand::ToTargetCommand(float speed, float minDistance)
    : speed_(speed)
    , minDistance_(minDistance) {
}

void ToTargetCommand::Execute(GameObject* executor, GameObject* target) {
    // executorまたはtargetがnullの場合は何もしない
    if (!executor || !target) {
        return;
    }

    // 現在位置とターゲット位置を取得
    Vector3 currentPos = executor->GetPosition();
    Vector3 targetPos = target->GetPosition();

    // ターゲットへの方向ベクトルを計算
    Vector3 direction = targetPos - currentPos;
    float distance = direction.Length();

    // 最小距離より近い場合は停止
    if (distance <= minDistance_) {
        executor->SetVelocity({0.0f, 0.0f, 0.0f});
        return;
    }

    direction.y = 0;

    // 方向ベクトルを正規化してvelocityを設定
    direction = direction.Normalize();
    executor->SetVelocity(direction * speed_);
}
