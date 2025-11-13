#include "GameScene.hpp"

#include "Collision/CollisionManager.h"
#include "Pattern/Singleton.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"

void GameScene::Initialize() {
    name_ = "game";

    entryTransition_ = Transition::Type::Fade;

    cManager_ = Singleton<Collision::Manager>::GetInstance();

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
        if (!stage_->GetPlayer()->IsActive() && !outro_) {
            next_ = "gameover";
            postEffects_->ApplyPreset("DarkScene", "replace", {}, [this]() {
                Change();
            });
            outro_ = true;
        }
    }

    followCamera_->SetActive(intro_->IsCameraDone());

    stage_->Update();
    followCamera_->Update();

    cManager_->Detect();
    cManager_->ProcessEvent();
}

void GameScene::Draw() {
    stage_->Draw();

    if (!introD_) {
        intro_->Draw();
    }
}
