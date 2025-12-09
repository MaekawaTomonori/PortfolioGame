#include "InputHandler.hpp"

#include "Camera/Controller/CameraController.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Camera.hpp"
#include "Math/MathUtils.hpp"

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
        const auto& camera_ = Singleton<CameraController>::GetInstance()->GetActive();

        // スクリーン座標を取得
        Vector2 screenPos = input_->GetMousePosition();

        // 画面サイズ
        float screenW = 1280.0f;
        float screenH = 720.0f;

        // スクリーン座標をNDC座標に変換
        float ndcX = (screenPos.x / screenW) * 2.0f - 1.0f;
        float ndcY = -((screenPos.y / screenH) * 2.0f - 1.0f);  // Y軸反転

        // View/Projection行列の取得と逆行列
        Matrix4x4 matView = camera_->GetView();
        Matrix4x4 matProj = camera_->GetProjection();
        Matrix4x4 matVP = matView * matProj;
        Matrix4x4 matInvVP = matVP.Inverse();

        // 近平面と遠平面の点（NDC空間）
        Vector3 nearClip = {ndcX, ndcY, 0.0f};
        Vector3 farClip = {ndcX, ndcY, 1.0f};

        // クリップ空間からワールド空間へ変換
        auto UnprojectPoint = [&](const Vector3& clip) -> Vector3 {
            float x = clip.x;
            float y = clip.y;
            float z = clip.z;
            float w = 1.0f;

            float wx = matInvVP.matrix[0][0] * x + matInvVP.matrix[0][1] * y + matInvVP.matrix[0][2] * z + matInvVP.matrix[0][3] * w;
            float wy = matInvVP.matrix[1][0] * x + matInvVP.matrix[1][1] * y + matInvVP.matrix[1][2] * z + matInvVP.matrix[1][3] * w;
            float wz = matInvVP.matrix[2][0] * x + matInvVP.matrix[2][1] * y + matInvVP.matrix[2][2] * z + matInvVP.matrix[2][3] * w;
            float ww = matInvVP.matrix[3][0] * x + matInvVP.matrix[3][1] * y + matInvVP.matrix[3][2] * z + matInvVP.matrix[3][3] * w;

            if (std::abs(ww) > 0.0001f) {
                return {wx / ww, wy / ww, wz / ww};
            }
            return {wx, wy, wz};
        };

        // Rayの始点と終点を計算
        Vector3 rayNear = UnprojectPoint(nearClip);
        Vector3 rayFar = UnprojectPoint(farClip);
        Vector3 rayDir = (rayFar - rayNear).Normalize();

        // プレイヤーのY座標平面との交差判定
        float planeY = currentPos.y;

        if (std::abs(rayDir.y) > 0.0001f) {
            float t = (planeY - rayNear.y) / rayDir.y;

            if (t >= 0.0f) {
                // 交点を計算
                Vector3 hitPos = rayNear + rayDir * t;

                // プレイヤーから交点への方向ベクトル（Y軸を無視）
                Vector3 direction = hitPos - currentPos;
                direction.y = 0.0f;

                if (direction.Length() > 0.001f) {
                    context.targetPosition = hitPos;
                } else {
                    // 交点がプレイヤー位置とほぼ同じ場合はデフォルト方向
                    context.targetPosition = currentPos + Vector3{0.0f, 0.0f, 1.0f};
                }
            } else {
                // Rayが後ろ向きの場合はデフォルト
                context.targetPosition = currentPos + Vector3{0.0f, 0.0f, 1.0f};
            }
        } else {
            // Rayがほぼ水平の場合はデフォルト
            context.targetPosition = currentPos + Vector3{0.0f, 0.0f, 1.0f};
        }
    }
}
