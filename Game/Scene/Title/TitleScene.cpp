#include "TitleScene.hpp"

#include "IGame.hpp"
#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

void TitleScene::Initialize() {
    stage_ = std::make_unique<Stage>();
    stage_->Initialize();

    titleLogo_ = std::make_unique<Sprite>();
    titleLogo_->Initialize("title.png");
    titleLogo_->SetPosition({ 640.f, 360.f });
    titleLogo_->SetAnchorPoint({.5f, .5f});

    pushtoStart_ = std::make_unique<Sprite>();
    pushtoStart_->Initialize("space.png");
    pushtoStart_->SetAnchorPoint({.5f, 0.5f});
    pushtoStart_->SetPosition({ 640.f, 500.f });

    Singleton<CameraDirector>::GetInstance()->Run("title", true);

    //PostProcessExecutor* postProcessor_ = game_->GetPostProcessor();
    //postProcessor_->Add(std::make_unique<Grayscale>(),"Grayscale");
    //postProcessor_->Add(std::make_unique<Vignette>(), "Vignette");
    //postProcessor_->Add(std::make_unique<BoxBlur>(), "BoxBlur");
}

void TitleScene::Update() {
    if (Singleton<Input>::GetInstance()->IsPress(DIK_SPACE)) {
        // Start the game
        next_ = "sample";
        Change();
    }

    stage_->Update();
    titleLogo_->Update();

    alpha_ += increase_ ? 0.02f : -0.02f;
    if (alpha_ >= 1.f) { alpha_ = 1.f; increase_ = false;}
    else if (alpha_ <= 0.f) {alpha_ = 0.f; increase_ = true;}

    Vector4 color = pushtoStart_->GetColor();
    color.w = alpha_;
    pushtoStart_->SetColor(color);
    pushtoStart_->Update();
}

void TitleScene::Draw() {
    stage_->Draw();
    titleLogo_->Draw();
    pushtoStart_->Draw();
}

void TitleScene::Finalize() {
    IScene::Finalize();
}
