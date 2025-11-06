#include "Enemy.hpp"

Enemy::Enemy()
    : moveCommand_(nullptr)
    , target_(nullptr)
    , model_(nullptr) {
}

Enemy::~Enemy() {
}

void Enemy::Initialize() {
    model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
    model_->SetEnvironmentTexture("Skybox.dds");
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;

    // 移動コマンドを実行
    if (moveCommand_ && target_) {
        moveCommand_->Execute(this, target_, deltaTime);
    }

    // モデル更新
    if (model_) {
        model_->SetScale(scale_);
        model_->SetRotate(rotation_);
        model_->SetTranslate(position_);
        model_->Update();
    }
}

void Enemy::Draw() {
    if (!active_ || !model_) return;

    model_->Draw();
}