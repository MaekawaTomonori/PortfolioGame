#include "Player.hpp"

Player::Player() : GameObject() {
}

Player::~Player() {
}

void Player::Initialize() {
    // MovementComponentの初期化
    movement_ = std::make_unique<MovementComponent>();
    movement_->Initialize();
    
    // モデルの初期化
    model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
    model_->SetEnvironmentTexture("skybox.dds");
    
    // 初期位置設定
    SetPosition({0.0f, 0.0f, 0.0f});
    SetScale({1.0f, 1.0f, 1.0f});
}

void Player::Update(float deltaTime) {
    if (!active_ || !movement_) return;
    
    // MovementComponentから移動量を取得して位置を更新
    Vector3 deltaMovement = movement_->Update(deltaTime);
    Vector3 newPosition = position_ + deltaMovement;
    SetPosition(newPosition);
    
    // トランスフォーム更新
    UpdateTransform();

    // モデル更新
    if (model_) {
        model_->SetScale(scale_);
        model_->SetRotate(rotation_);
        model_->SetTranslate(position_);
        model_->Update();
    }
}

void Player::Draw() {
    if (!active_ || !model_) return;
    
    model_->Draw();
}

void Player::SetMoveSpeed(float speed) {
    if (movement_) {
        movement_->SetMoveSpeed(speed);
    }
}

const Vector3& Player::GetVelocity() const {
    if (movement_) {
        return movement_->GetVelocity();
    }
    static Vector3 zero{0.0f, 0.0f, 0.0f};
    return zero;
}