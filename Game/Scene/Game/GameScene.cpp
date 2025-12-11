#include "GameScene.hpp"

#include "Collision/CollisionManager.h"
#include "Light/LightManager.hpp"
#include "Pattern/Singleton.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"

void GameScene::Initialize() {
    name_ = "game";

    entryTransition_ = Transition::Type::Fade;

    cManager_ = Singleton<Collision::Manager>::GetInstance();

    stage_ = std::make_unique<Stage>();
    stage_->Initialize(Particle(), PostEffect());

    followCamera_ = std::make_unique<FollowCamera>();
    followCamera_->Initialize();
    followCamera_->SetActive(true);
    followCamera_->SetTarget(stage_->GetPlayer());
    followCamera_->SetEnemies(stage_->GetEnemies());

    stage_->SetCamera(followCamera_.get());

    //intro_ = std::make_unique<Intro>();
    //intro_->Initialize();
}

void GameScene::Update() {
    //if (!introD_) {
    //    intro_->Update();
    //    introD_ = intro_->IsFinish();
    //    followCamera_->SetActive(intro_->IsCameraDone());
    //}
    //
    //if (introD_ && !clear_ && !outro_) {
    //    if (!stage_->GetPlayer()->IsActive() && !outro_) {
    //        next_ = "gameover";
    //        PostEffect()->ApplyPreset("DarkScene", "replace", {}, [this]() {
    //            Change();
    //            });
    //        outro_ = true;
    //    }
    //
    //    if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
    //        clear_ = true;
    //        outro_ = true;
    //        outroAnim_ = std::make_unique<Outro>();
    //        outroAnim_->Run();
    //    }
    //}
    //
    //if (clear_ && !outro_) {
    //    next_ = "gameclear";
    //    Change();
    //}
    //
    //if (clear_ && outro_) {
    //    followCamera_->SetActive(false);
    //    outro_ = !outroAnim_->IsFinish();
    //}

    stage_->Update();
    followCamera_->Update();

    cManager_->Detect();
    cManager_->ProcessEvent();
}

void GameScene::Draw() {
    stage_->Draw();

    //if (!introD_) {
    //    intro_->Draw();
    //}
}

void GameScene::Debug() {
    followCamera_->Debug();
    stage_->Debug();
}
