#include "GameScene.hpp"

#include "Camera/Director/CameraDirector.hpp"

void GameScene::Initialize() {
    name_ = "game";

    entryTransition_ = Transition::Type::Fade;

    camerawork_ = Singleton<CameraDirector>::GetInstance();
    stage_ = std::make_unique<Stage>();
    stage_->Initialize();

    followCamera_ = std::make_unique<FollowCamera>();
    followCamera_->Initialize();
    followCamera_->SetDebug(debug_);
    followCamera_->SetTarget(stage_->GetPlayer());

    Singleton<CameraController>::GetInstance()->GetActive()->transform_.translate = { 0.f, 5.f, -5.f };
}

void GameScene::Update() {
    if (!camerawork_)return;
    if (intro_){
        camerawork_->Run("intro");
        intro_ = false;
    }

    followCamera_->SetActive(!camerawork_->IsPlaying());

    stage_->Update();
    followCamera_->Update();
}

void GameScene::Draw() {
    stage_->Draw();
}
