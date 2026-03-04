#include "TitleScene.hpp"
#include "Input.hpp"
#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

void TitleScene::Initialize() {
    name_ = "title";

    exitTransition_ = Transition::Type::Fade;
    
    status_ = {
        .point = 0,
        .time = 15,
        .requirementKill = 10,
        .maxEnemyCount = 1,
        .enemySpawnInterval = 0.f,
        .playerStatus = {
            .hp = 10.f,
            .damage = 1.f,
            .ms = 1.f,
            .as = 1.f
        }
    };

    stage_ = std::make_unique<Stage>(status_);
    stage_->Setup(Particle(), PostEffect());
    stage_->Initialize();
    stage_->SetActive(false);

    titleLogo_ = std::make_unique<Sprite>();
    titleLogo_->Initialize("title.png");
    titleLogo_->SetPosition({ 640.f, 360.f });
    titleLogo_->SetAnchorPoint({ .5f, .5f });

    pushtoStart_ = std::make_unique<Sprite>();
    pushtoStart_->Initialize("space.png");
    pushtoStart_->SetAnchorPoint({ .5f, 0.5f });
    pushtoStart_->SetPosition({ 640.f, 500.f });

    //PostProcessExecutor* postProcessor_ = game_->GetPostProcessor();
    //postProcessor_->Add(std::make_unique<Grayscale>(),"Grayscale");
    //postProcessor_->Add(std::make_unique<Vignette>(), "Vignette");
    //postProcessor_->Add(std::make_unique<BoxBlur>(), "BoxBlur");
}

void TitleScene::Update() {
    if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
        // Start the game
        next_ = "game";
        Change();
    }

    stage_->Update();

    alpha_ += increase_ ? 0.02f : -0.02f;
    if (alpha_ >= 1.f) { alpha_ = 1.f; increase_ = false; }
    else if (alpha_ <= 0.f) { alpha_ = 0.f; increase_ = true; }

    Vector4 color = pushtoStart_->GetColor();
    color.w = alpha_;
    pushtoStart_->SetColor(color);
}

void TitleScene::Draw() {
    stage_->Draw();
    titleLogo_->Draw();
    pushtoStart_->Draw();
}

void TitleScene::Debug() {
    stage_->Debug();
}

void TitleScene::Finalize() {
    IScene::Finalize();
}

void TitleScene::OnEnable() {
    Singleton<CameraDirector>::GetInstance()->Run("cp", true);
}
