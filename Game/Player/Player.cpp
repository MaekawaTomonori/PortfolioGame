#include "Player.hpp"
#include "Camera/Manager/CameraManager.hpp"
#include "Pattern/Singleton.hpp"

Player::Player() : GameObject() {
}

Player::~Player() {
}

void Player::Initialize() {
    input_ = Singleton<Input>::GetInstance();
    cameraManager_ = Singleton<CameraManager>::GetInstance();
    
    model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
    model_->SetEnvironmentTexture("skybox.dds");
    
    // 初期位置設定
    SetPosition({0.0f, 0.0f, 0.0f});
    SetScale({1.0f, 1.0f, 1.0f});
}

void Player::Update(float deltaTime) {
    if (!active_) return;
    
    UpdateMovement(deltaTime);
    UpdateCameraOffset(deltaTime);
    
    // トランスフォーム更新
    UpdateTransform();

    if (!model_)return;
    model_->SetTranslate(position_);
    model_->SetRotate(rotation_);
    model_->SetScale(scale_);
    model_->Update();
}

void Player::Draw() {
    if (!active_ || !model_) return;
    
    model_->Draw();
}

void Player::UpdateMovement(float deltaTime) {
    // 入力ベクトル取得
    Vector3 inputVector = GetInputVector();
    
    // 移動速度適用
    velocity_ = inputVector * moveSpeed_;
    
    // 位置更新
    Vector3 newPosition = position_ + velocity_ * deltaTime;
    SetPosition(newPosition);
}

void Player::UpdateCameraOffset(float deltaTime) {
    if (!cameraManager_) return;
    
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
                cameraOffset_ = {0.0f, 0.0f, 0.0f};
            } else {
                cameraOffset_ += returnVector;
            }
        }
    }
    
    // カメラ位置をプレイヤー位置 + オフセットに設定
    Vector3 cameraPos = position_ + Vector3{cameraOffset_.x, 10.0f, cameraOffset_.z};
    cameraManager_->GetActive()->transform_.translate = cameraPos;
    
    Vector3 cameraRotation = {90.0f * (3.14159f / 180.0f), 0.0f, 0.0f}; // 90度下向き
    cameraManager_->GetActive()->transform_.rotate = cameraRotation;
}

Vector3 Player::GetInputVector() const {
    if (!input_) return {0.0f, 0.0f, 0.0f};
    
    Vector3 inputVector{0.0f, 0.0f, 0.0f};
    
    // WASD入力
    if (input_->IsPress(DIK_W)) inputVector.z += 1.0f;
    if (input_->IsPress(DIK_S)) inputVector.z -= 1.0f;
    if (input_->IsPress(DIK_A)) inputVector.x -= 1.0f;
    if (input_->IsPress(DIK_D)) inputVector.x += 1.0f;
    
    if (inputVector.Length() > 0.0f) {
        inputVector = inputVector.Normalize();
    }
    
    return inputVector;
}

Vector3 Player::GetCameraOffsetInput() const {
    if (!input_) return {0.0f, 0.0f, 0.0f};
    
    Vector3 offsetInput{0.0f, 0.0f, 0.0f};
    
    if (input_->IsPress(DIK_UP)) offsetInput.z += 1.0f;
    if (input_->IsPress(DIK_DOWN)) offsetInput.z -= 1.0f;
    if (input_->IsPress(DIK_LEFT)) offsetInput.x -= 1.0f;
    if (input_->IsPress(DIK_RIGHT)) offsetInput.x += 1.0f;
    
    return offsetInput;
}