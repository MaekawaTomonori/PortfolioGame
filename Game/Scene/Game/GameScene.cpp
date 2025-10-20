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

    //Singleton<CameraController>::GetInstance()->GetActive()->transform_.translate = { 0.f, 5.f, -5.f };
}

void GameScene::Update() {
    if (!camerawork_)return;

    if (Singleton<Input>::GetInstance()->IsPress(DIK_SPACE)){
        camerawork_->Run("intro");
    }

    followCamera_->SetActive(!camerawork_->IsPlaying());

    stage_->Update();
    followCamera_->Update();
}

void GameScene::Draw() {
    stage_->Draw();
}
