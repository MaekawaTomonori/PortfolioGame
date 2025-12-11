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
    context.isDashRequested = input_->IsPress(DIK_LSHIFT);

    // フラッシュ (Fキー)
    context.isFlashRequested = input_->IsTrigger(DIK_F);

    if (context.isFlashRequested) {
        context.moveDirection = (context.position - GetMouseInWorld()).Normalize();
    }
}

Vector3 InputHandler::GetMouseInWorld() {
    if (!input_) return {};
    if (auto camera = Singleton<CameraController>::GetInstance()->GetActive()) {
        // スクリーン座標を取得
        Vector2 mousePos = input_->GetMousePosition();
        // 正規化デバイス座標に変換
        
    }
    return {};
}
