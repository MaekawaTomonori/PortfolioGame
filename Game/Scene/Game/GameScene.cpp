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
        .requirementKill = 5,
        .maxEnemyCount = 5,
        .enemySpawnInterval = 2.f,
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

    followCamera_ = std::make_unique<FollowCamera>();
    followCamera_->Initialize();
    followCamera_->SetTarget(stage_->GetPlayer());
    followCamera_->SetEnemies(stage_->GetEnemies());
    followCamera_->SetActive(false);

    stage_->SetCamera(followCamera_.get());
    stage_->SetActive(false);

    intro_ = std::make_unique<Intro>();

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
    
    killCounter_ = std::make_unique<KillCounter>();
    
    skillTree_ = std::make_unique<SkillTree>();
    skillTree_->Initialize(&status_);
    skillTree_->SetOnContinue([this] {
        PlayTransition(Transition::Type::Fade, [this] {
            gameTimer_->SetDuration(status_.time);
            stage_->Initialize();
            killCounter_->Initialize(stage_->GetEnemies(), status_.requirementKill);
            killCounter_->Update();
            state_ = PLAY;
            skillTree_->Close();
        });
    });
}

void GameScene::Update() {
    pause_->Update();

    if (pause_->IsOpen())return;

    switch (state_) {
    case INTRO:
        if (!intro_)break;
        intro_->Update();
        followCamera_->Update();

        if (intro_->IsCameraDone()){
            followCamera_->SetActive(true);
        }
        if (intro_->IsFinish()) {
            state_ = PLAY;
            stage_->SetActive(true);
            killCounter_->Initialize(stage_->GetEnemies(), status_.requirementKill);
            UpdatePlay();
        }
        break;
    case PLAY:
        UpdatePlay();
        break;
    case UPGRADE:
        skillTree_->Update();
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
        killCounter_->Draw();
        break;
    case UPGRADE:
        skillTree_->Draw();
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
    keyGuide_->Debug();
    skillTree_->Debug();
    killCounter_->Debug();

    ImGui::Begin("Status");
    ImGui::DragFloat("Timer", &status_.time, 1.f, 0.f, 300.f);
    int tmp = static_cast<int>(status_.maxEnemyCount);
    if (ImGui::DragInt("Max Enemy Count", &tmp, 1, 1, 100)){
        status_.maxEnemyCount = static_cast<uint16_t>(tmp);
    }
    ImGui::DragFloat("Enemy Spawn Interval", &status_.enemySpawnInterval, 0.1f, 0.f, 10.f);
    ImGui::End();
}

void GameScene::OnEnable() {
    intro_->Initialize();
}

void GameScene::UpdatePlay() {
    gameTimer_->Update(1.f / 60.f);
    stage_->Update();
    killCounter_->Update();
    followCamera_->Update();
    keyGuide_->Update();
    cManager_->Detect();
    cManager_->ProcessEvent();

    if (gameTimer_->IsDone()) {
        status_.point += stage_->GetEnemies()->GetDeathCount();
        PlayTransition(Transition::Type::Fade, [this] {
            state_ = UPGRADE;
            skillTree_->Open();
            skillTree_->Update();
            });
    }

    // is clear
    if (stage_->IsClear() || Singleton<Input>::GetInstance()->IsTrigger(DIK_T)) {
        state_ = OUTRO;
        PostEffect()->ApplyPreset("DarkScene", "replace", {}, [&] {
            next_ = "gameclear";
            Change();
        });
    }
}
