#include "InputHandler.hpp"

#include "Camera/Controller/CameraController.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Camera.hpp"

void InputHandler::Initialize() {
    input_ = Singleton<Input>::GetInstance();
}

void InputHandler::UpdateContext(MovementContext& context, const Vector3& currentPos) {
    if (!input_) return;

    context.Reset();
    context.position = currentPos;
    context.targetPosition = GetMouseInWorld();

    // 移動入力
    if (input_->IsPress(DIK_W)) context.moveDirection.z += 1.0f;
    if (input_->IsPress(DIK_S)) context.moveDirection.z -= 1.0f;
    if (input_->IsPress(DIK_A)) context.moveDirection.x -= 1.0f;
    if (input_->IsPress(DIK_D)) context.moveDirection.x += 1.0f;

    // 正規化
    if (context.moveDirection.Length() > 0.0f) {
        context.moveDirection = context.moveDirection.Normalize();
    }

    // ダッシュ
    context.isDashRequested = input_->IsPress(DIK_SPACE);

    // フラッシュ
    context.isFlashRequested = input_->IsTrigger(DIK_F);

    // スキル (Qキー)
    context.isSkillRequested = input_->IsTrigger(DIK_Q);
}

Vector3 InputHandler::GetMouseInWorld() const {
    if (!input_) return {};
    if (auto camera = Singleton<CameraController>::GetInstance()->GetActive()) {
        // スクリーン座標を取得
        Vector2 mousePos = input_->GetMousePosition();
        
        // スクリーン座標 → NDC に手動変換
        float ndcX = (2.0f * mousePos.x / Viewport.x) - 1.0f;
        float ndcY = 1.0f - (2.0f * mousePos.y / Viewport.y);

        // ViewProjectionの逆行列のみで逆変換
        Matrix4x4 mInv = camera->GetViewProjection().Inverse();

        Vector3 nPos = {ndcX, ndcY, 0.f};
        Vector3 fPos = {ndcX, ndcY, 1.f};

        nPos = MathUtils::Matrix::Transform(nPos, mInv);
        fPos = MathUtils::Matrix::Transform(fPos, mInv);

        Vector3 mouseDir = (fPos - nPos).Normalize();

        Vector3 res = camera->transform_.translate;

        // レイとY=0平面の交点を求める
        if (std::abs(mouseDir.y) > 1e-6f) {
            float t = -nPos.y / mouseDir.y;
            if (t >= 0.f) {
                res = nPos + mouseDir * t;
            }
        }
        return res;
    }
    return {};
}
