#include "TitleScene.hpp"

#include "IGame.hpp"
#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

#include "PostProcess/BoxBlur/BoxBlur.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"
#include "PostProcess/Grayscale/Grayscale.hpp"
#include "PostProcess/Vignette/Vignette.hpp"

void TitleScene::Initialize() {
    stage_ = std::make_unique<Stage>();
    stage_->Initialize();

    Singleton<CameraDirector>::GetInstance()->Run("title", true);

    PostProcessExecutor* postProcessor_ = game_->GetPostProcessor();
    postProcessor_->Add(std::make_unique<Grayscale>(),"Grayscale");
    postProcessor_->Add(std::make_unique<Vignette>(), "Vignette");
    postProcessor_->Add(std::make_unique<BoxBlur>(), "BoxBlur");
}

void TitleScene::Update() {
    stage_->Update();
}

void TitleScene::Draw() {
    stage_->Draw();
}

void TitleScene::Finalize() {
    IScene::Finalize();
}
