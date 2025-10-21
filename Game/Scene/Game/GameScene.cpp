#include "GameScene.hpp"

#include "Camera/Director/CameraDirector.hpp"

void GameScene::Initialize() {
    name_ = "game";

    entryTransition_ = Transition::Type::Fade;

    stage_ = std::make_unique<Stage>();
    stage_->Initialize();

    followCamera_ = std::make_unique<FollowCamera>();
    followCamera_->Initialize();
    followCamera_->SetDebug(debug_);
    followCamera_->SetTarget(stage_->GetPlayer());

    intro_ = std::make_unique<Intro>();
    intro_->Initialize();
}

void GameScene::Update() {
    if (!introD_){
        intro_->Update();
        introD_ = intro_->IsFinish();
    }

    if (introD_) {
        if (Singleton<Input>::GetInstance()->IsPress(DIK_SPACE)) {
            intro_->Initialize();
            introD_ = false;
        }
    }

    followCamera_->SetActive(intro_->IsCameraDone());

    stage_->Update();
    followCamera_->Update();
}

void GameScene::Draw() {
    stage_->Draw();

    if (!introD_) {
        intro_->Draw();
    }
}
