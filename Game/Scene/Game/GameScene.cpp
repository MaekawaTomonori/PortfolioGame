#include "GameScene.hpp"

#include "imgui_internal.h"
#include "Collision/CollisionManager.h"
#include "Pattern/Singleton.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"

void GameScene::Initialize() {
    name_ = "game";

    entryTransition_ = Transition::Type::Fade;

    cManager_ = Singleton<Collision::Manager>::GetInstance();

    status_ ={
        .point = 0,
        .time = 15,
        .maxEnemyCount = 5,
        .enemySpawnInterval = 2.f,
        .playerStatus = {
            .hp = 10.f,
            .damage = 1.f,
            .ms = 1.f,
            .as = 1.f
        }
    };

    stage_ = std::make_unique<Stage>();
    stage_->Setup(Particle(), PostEffect());
    stage_->Initialize(status_);

    followCamera_ = std::make_unique<FollowCamera>();
    followCamera_->SetTarget(stage_->GetPlayer());
    followCamera_->Initialize();
    followCamera_->SetEnemies(stage_->GetEnemies());
    followCamera_->SetActive(false);

    stage_->SetCamera(followCamera_.get());

    intro_ = std::make_unique<Intro>();
    intro_->Initialize();

    outro_ = std::make_unique<Outro>();

    gameTimer_ = std::make_unique<GameTimer>();
    gameTimer_->Initialize();
    gameTimer_->SetDuration(status_.time);
    gameTimer_->SetPosition({640.f, 69.f});

    pause_ = std::make_unique<Pause>();
    pause_->Initialize();
    pause_->SetOnRetry([this]{ next_ = "game"; Change(); });
    pause_->SetOnQuit([this]{ next_ = "title"; Change(); });

    keyGuide_ = std::make_unique<KeyGuide>();
    keyGuide_->Initialize();

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("wip.png");
    sprite_->SetPosition({640.f, 360.f});
}

void GameScene::Update() {
    pause_->Update();

    if (pause_->IsOpen())return;

    switch (state_) {
    case INTRO:
        if (!intro_)break;
        intro_->Update();
        followCamera_->Update();
        if (intro_->IsFinish()) {
            state_ = PLAY;
            followCamera_->SetActive(true);
        }
        break;
    case PLAY:
        gameTimer_->Update(1.f/ 60.f);
        stage_->Update();
        followCamera_->Update();
        keyGuide_->Update();
        cManager_->Detect();
        cManager_->ProcessEvent();

        if (gameTimer_->IsDone()) {
             state_ = UPGRADE;
        }

        // is clear
        if (stage_->IsClear()) {
            state_ = OUTRO;
            PostEffect()->ApplyPreset("DarkScene", "replace", {}, [&] {
                next_ = "gameclear";
                Change();
            });
        }

        break;
    case UPGRADE:
        if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
            gameTimer_->SetDuration(status_.time);
            stage_->Initialize(status_);
            state_ = PLAY;
        }
        sprite_->Update();
        break;
    default: ;
    }
}

void GameScene::Draw() {
    switch (state_) {
    case INTRO:
        intro_->Draw();
        stage_->Draw();
        break;
    case PLAY:
        gameTimer_->Draw();
        stage_->Draw();
        keyGuide_->Draw();
        break;
    case UPGRADE:
        sprite_->Draw();
        break;
    case OUTRO:
        stage_->Draw();
        break;
    default: ;
    }
    pause_->Draw();
}

void GameScene::Debug() {
    followCamera_->Debug();
    gameTimer_->Debug();
    stage_->Debug();
    pause_->Debug();

    ImGui::Begin("Status");
    ImGui::DragFloat("Timer", &status_.time, 1.f, 0.f, 300.f);
    int tmp = static_cast<int>(status_.maxEnemyCount);
    if (ImGui::DragInt("Max Enemy Count", &tmp, 1, 1, 100)){
        status_.maxEnemyCount = static_cast<uint16_t>(tmp);
    }
    ImGui::DragFloat("Enemy Spawn Interval", &status_.enemySpawnInterval, 0.1f, 0.f, 10.f);
    ImGui::End();
}
