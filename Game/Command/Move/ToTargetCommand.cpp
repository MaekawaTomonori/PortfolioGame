#include "ToTargetCommand.hpp"
#include "GameObject/GameObject.hpp"

ToTargetCommand::ToTargetCommand(float speed, float minDistance)
    : speed_(speed)
    , minDistance_(minDistance) {
}

void ToTargetCommand::Execute(GameObject* executor, GameObject* target, float deltaTime) {
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

    // 最小距離より近い場合は移動しない
    if (distance <= minDistance_) {
        return;
    }

    // 方向ベクトルを正規化して移動量を計算
    direction = direction.Normalize();
    Vector3 movement = direction * speed_ * deltaTime;

    // 新しい位置を設定
    executor->SetPosition(currentPos + movement);
}
