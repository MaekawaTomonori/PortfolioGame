#include "CameraController.hpp"
#include "GameObject/GameObject.hpp"
#include <dinput.h>

CameraController::CameraController() : 
    target_(nullptr), 
    input_(nullptr), 
    cameraManager_(nullptr) {
}

CameraController::~CameraController() {
}

void CameraController::Initialize() {
    input_ = Singleton<Input>::GetInstance();
    cameraManager_ = Singleton<CameraManager>::GetInstance();
}

void CameraController::Update(float deltaTime) {
    if (!target_ || !input_ || !cameraManager_) return;
    
    // カメラオフセット入力取得
    Vector3 offsetInput = GetCameraOffsetInput();
    
    if (offsetInput.Length() > 0.0f) {
        // 入力がある場合：オフセット更新
        cameraOffset_ += offsetInput * deltaTime * 10.0f; // 感度調整
        
        // 最大オフセット制限
        float currentLength = cameraOffset_.Length();
        if (currentLength > maxCameraOffset_) {
            cameraOffset_ = cameraOffset_.Normalize() * maxCameraOffset_;
        }
    } else {
        // 入力がない場合：中央に戻る
        if (cameraOffset_.Length() > 0.01f) {
            Vector3 returnVector = cameraOffset_ * -1.0f;
            returnVector = returnVector.Normalize() * cameraReturnSpeed_ * deltaTime;
            
            if (returnVector.Length() > cameraOffset_.Length()) {
                cameraOffset_ = {0.0f, 10.0f, 0.0f}; // デフォルトオフセット
            } else {
                cameraOffset_ += returnVector;
            }
        }
    }
    
    UpdateCameraPosition();
}

Vector3 CameraController::GetCameraOffsetInput() const {
    if (!input_) return {0.0f, 0.0f, 0.0f};
    
    Vector3 offsetInput{0.0f, 0.0f, 0.0f};
    
    if (input_->IsPress(DIK_UP)) offsetInput.z += 1.0f;
    if (input_->IsPress(DIK_DOWN)) offsetInput.z -= 1.0f;
    if (input_->IsPress(DIK_LEFT)) offsetInput.x -= 1.0f;
    if (input_->IsPress(DIK_RIGHT)) offsetInput.x += 1.0f;
    
    return offsetInput;
}

void CameraController::UpdateCameraPosition() {
    if (!target_ || !cameraManager_) return;
    
    // カメラ位置をターゲット位置 + オフセットに設定
    Vector3 targetPosition = target_->GetPosition();
    Vector3 cameraPos = targetPosition + Vector3{cameraOffset_.x, cameraHeight_, cameraOffset_.z};
    cameraManager_->GetActive()->transform_.translate = cameraPos;
    
    // カメラを下向きに設定（トップダウン視点）
    Vector3 cameraRotation = {90.0f * (3.14159f / 180.0f), 0.0f, 0.0f}; // 90度下向き
    cameraManager_->GetActive()->transform_.rotate = cameraRotation;
}