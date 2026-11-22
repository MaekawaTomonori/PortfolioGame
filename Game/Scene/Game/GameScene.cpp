#include "GameScene.hpp"

#include "imgui.h"
#include "HpBar/HpBar.hpp"
#include "Input.hpp"
#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"
#include "Debug/Debugger.hpp"

void GameScene::Initialize() {
    entryTransition_ = Transition::Type::Fade;

    status_.Load();
    HpBar::LoadSettings();
    binds_.Load();
    theme_.Load();

    stage_ = std::make_unique<Stage>(status_);
    stage_->Setup(Particle(), PostEffect(), binds_);
    stage_->Initialize();

    followCamera_ = std::make_unique<FollowCamera>();
    followCamera_->Initialize();
    followCamera_->SetTarget(stage_->GetPlayer());
    followCamera_->SetEnemies(stage_->GetEnemies());
    followCamera_->SetPlayer(stage_->GetPlayer());
    followCamera_->SetActive(false);

    stage_->SetCamera(followCamera_.get());
    stage_->SetActive(false);

    intro_ = std::make_unique<Intro>();

    hud_ = std::make_unique<PlayHud>();
    hud_->Initialize(binds_, theme_);

    pause_ = std::make_unique<Pause>();
    pause_->Initialize();
    pause_->SetOnRetry([this]{ next_ = "game"; Change(); });
    pause_->SetOnQuit([this]{ next_ = "title"; Change(); });

    skillTree_ = std::make_unique<SkillTree>();
    skillTree_->Initialize(&status_, theme_);
    skillTree_->SetOnContinue([this] {
        PlayTransition(Transition::Type::Fade, [this] {
            StartNextRound();
            skillTree_->Close();
        });
    });
#ifdef _DEBUG
    Debugger::WatchGroup("Stage")
        .Watch("Time",               &status_.time)
        .Watch("MaxEnemyCount",      &status_.maxEnemyCount)
        .Watch("RequirementKill",    &status_.requirementKill)
        .Watch("EnemySpawnInterval", &status_.enemySpawnInterval)
        .Watch("Point", &status_.point);

    Debugger::WatchGroup("Player")
        .Watch("HP",     &status_.playerStatus.hp)
        .Watch("Damage", &status_.playerStatus.damage)
        .Watch("MS",     &status_.playerStatus.ms)
        .Watch("AS",     &status_.playerStatus.as);
#endif
}

void GameScene::Update() {
    pause_->Update();
    UpdateCursorVisibility();

    if (pause_->IsOpen())return;

    switch (state_) {
    case INTRO:
        if (!intro_) break;
        intro_->Update();
        stage_->Update();
        hud_->Update();
        followCamera_->Update();

        if (intro_->IsCameraDone()) {
            followCamera_->SetActive(true);
        }
        if (intro_->IsFinish() && hud_->IsIntroFinished()) {
            state_ = PLAY;
            stage_->SetActive(true);
            hud_->Show();
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

void GameScene::Finalize() {
    // 次のシーンにはカーソルを出したまま渡さない
    if (cursorShown_) {
        Singleton<Input>::GetInstance()->SetCursorVisible(false);
        cursorShown_ = false;
    }
}

void GameScene::UpdateCursorVisibility() {
    // メニューはマウスでも操作するので、どちらかが開いている間だけカーソルを出す
    const bool menuOpen = pause_->IsOpen() || skillTree_->IsOpen();
    if (menuOpen == cursorShown_) return;

    Singleton<Input>::GetInstance()->SetCursorVisible(menuOpen);
    cursorShown_ = menuOpen;
}

void GameScene::Draw() {
    switch (state_) {
    case INTRO:
        stage_->Draw();
        break;
    case PLAY:
        stage_->Draw();
        break;
    case UPGRADE:
        break;
    case OUTRO:
        stage_->Draw();
        break;
    default: ;
    }
}

void GameScene::Debug() {
    followCamera_->Debug();
    stage_->Debug();
    skillTree_->Debug();
    hud_->Debug();
    HpBar::DebugSettings();
    theme_.Debug();

    ImGui::Begin("Status");
    if (ImGui::Button("Save to JSON")) {
        status_.Save();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Assets/Data/GameStatus/GameStatus.json");
    ImGui::End();
}

void GameScene::OnEnable() {
    intro_->Initialize();
    hud_->PlayIntro(status_.requirementKill, stage_->GetEnemies(), status_.time);
}

void GameScene::UpdatePlay() {
    stage_->Update();
    followCamera_->Update();
    hud_->Update();

    if (const auto* skillManager = stage_->GetSkillManager()) {
        hud_->SetCooldowns(*skillManager);
    }

    // is clear (タイマーより優先してチェック)
    bool isClear = stage_->IsClear();
#ifdef _DEBUG
    isClear |= Singleton<Input>::GetInstance()->IsTrigger(DIK_T);
#endif
    if (isClear) {
        state_ = OUTRO;
        hud_->Hide();
        PostEffect()->ApplyPreset("DarkScene", [&] {
            next_ = "gameclear";
            Change();
        });
    } else if (hud_->IsTimeUp()) {
        status_.point += stage_->GetEnemies()->GetDeathCount();
        PlayTransition(Transition::Type::Fade, [this] {
            state_ = UPGRADE;
            hud_->Hide();
            skillTree_->Open();
            skillTree_->Update();
        });
    }

    if (stage_->IsGameOver()) {
        state_ = OUTRO;
        hud_->Hide();
        PostEffect()->ApplyPreset("DarkScene", [&] {
            next_ = "gameover";
            Change();
        });
    }
}

void GameScene::StartNextRound() {
    stage_->Initialize();
    followCamera_->ResetToTarget();
    state_ = PLAY;

    hud_->StartRound(status_.requirementKill, stage_->GetEnemies(), status_.time);
    hud_->Show();
}
