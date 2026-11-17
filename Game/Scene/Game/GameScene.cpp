#include "GameScene.hpp"

#include "imgui.h"
#include "HpBar/HpBar.hpp"
#include "Input.hpp"
#include "Log.hpp"
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"
#include "Debug/Debugger.hpp"

void GameScene::Initialize() {
    entryTransition_ = Transition::Type::Fade;

    status_ ={
        .point = 0,
        .time = 15,
        .requirementKill = 5,
        .maxEnemyCount = 100,
        .enemySpawnInterval = 2.f,
        .playerStatus = {
            .hp = 10.f,
            .damage = 1.f,
            .ms = 1.f,
            .as = 1.f
        }
    };

    LoadStatus();
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

    gameTimer_ = std::make_unique<GameTimer>();
    gameTimer_->Initialize(theme_);
    gameTimer_->SetDuration(status_.time);

    pause_ = std::make_unique<Pause>();
    pause_->Initialize();
    pause_->SetOnRetry([this]{ next_ = "game"; Change(); });
    pause_->SetOnQuit([this]{ next_ = "title"; Change(); });

    keyGuide_ = std::make_unique<KeyGuide>();
    keyGuide_->Initialize(binds_, theme_);

    killCounter_ = std::make_unique<KillCounter>();
    killCounter_->Initialize(theme_);

    stageInfoUI_ = std::make_unique<StageInfoUI>();
    stageInfoUI_->Initialize(theme_);
    // Objective が左上へ動き始めたら、その下のキルカウンターをその場でフェードインさせる
    stageInfoUI_->SetOnMoveStart([this] {
        killCounter_->Setup(stage_->GetEnemies(), status_.requirementKill);
        killCounter_->PlayIntroAnim();
    });

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
        stageInfoUI_->Update();
        killCounter_->Update();
        followCamera_->Update();

        if (intro_->IsCameraDone()) {
            followCamera_->SetActive(true);
        }
        if (intro_->IsFinish() && stageInfoUI_->IsFinish()) {
            state_ = PLAY;
            stage_->SetActive(true);
            ShowPlayHud();
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
    gameTimer_->Debug();
    stage_->Debug();
    skillTree_->Debug();
    keyGuide_->Debug();
    killCounter_->Debug();
    HpBar::DebugSettings();
    theme_.Debug();

    ImGui::Begin("Status");
    if (ImGui::Button("Save to JSON")) {
        SaveStatus();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Assets/Data/GameStatus/GameStatus.json");
    ImGui::End();
}

void GameScene::LoadStatus() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("GameStatus")) return;

    status_.time               = std::get<float>  (json->GetValue("GameStatus", "Stage",  "Time"));
    status_.maxEnemyCount      = static_cast<uint16_t>(std::get<int32_t>(json->GetValue("GameStatus", "Stage",  "MaxEnemyCount")));
    status_.requirementKill    = static_cast<uint16_t>(std::get<int32_t>(json->GetValue("GameStatus", "Stage",  "RequirementKill")));
    status_.enemySpawnInterval = std::get<float>  (json->GetValue("GameStatus", "Stage",  "EnemySpawnInterval"));
    status_.playerStatus.hp    = std::get<float>  (json->GetValue("GameStatus", "Player", "HP"));
    status_.playerStatus.damage= std::get<float>  (json->GetValue("GameStatus", "Player", "Damage"));
    status_.playerStatus.ms    = std::get<float>  (json->GetValue("GameStatus", "Player", "MS"));
    status_.playerStatus.as    = std::get<float>  (json->GetValue("GameStatus", "Player", "AS"));
}

void GameScene::SaveStatus() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->SetValue("GameStatus", "Stage",  "Time",               status_.time);
    json->SetValue("GameStatus", "Stage",  "MaxEnemyCount",      static_cast<int32_t>(status_.maxEnemyCount));
    json->SetValue("GameStatus", "Stage",  "RequirementKill",    static_cast<int32_t>(status_.requirementKill));
    json->SetValue("GameStatus", "Stage",  "EnemySpawnInterval", status_.enemySpawnInterval);
    json->SetValue("GameStatus", "Player", "HP",                 status_.playerStatus.hp);
    json->SetValue("GameStatus", "Player", "Damage",             status_.playerStatus.damage);
    json->SetValue("GameStatus", "Player", "MS",                 status_.playerStatus.ms);
    json->SetValue("GameStatus", "Player", "AS",                 status_.playerStatus.as);
    json->Save("GameStatus");
}

void GameScene::OnEnable() {
    intro_->Initialize();
    stageInfoUI_->Setup(status_.requirementKill);
}

void GameScene::UpdatePlay() {
    gameTimer_->Update();
    stage_->Update();
    // 左上に着いた直後は、まだ小さい文字のフェードインが続いている
    stageInfoUI_->Update();
    killCounter_->Update();
    followCamera_->Update();
    keyGuide_->Update();

    if (const auto* player = stage_->GetPlayer()) {
        keyGuide_->SetCooldown(InputAction::Ability2, player->GetFlashCooldownRatio());
    }
    if (const auto* sm = stage_->GetSkillManager()) {
        keyGuide_->SetCooldown(InputAction::Ability1, sm->GetBlackHoleCooldownRatio());
    }

    // is clear (タイマーより優先してチェック)
    bool isClear = stage_->IsClear();
#ifdef _DEBUG
    isClear |= Singleton<Input>::GetInstance()->IsTrigger(DIK_T);
#endif
    if (isClear) {
        state_ = OUTRO;
        HidePlayHud();
        PostEffect()->ApplyPreset("DarkScene", [&] {
            next_ = "gameclear";
            Change();
        });
    } else if (gameTimer_->IsDone()) {
        status_.point += stage_->GetEnemies()->GetDeathCount();
        PlayTransition(Transition::Type::Fade, [this] {
            state_ = UPGRADE;
            HidePlayHud();
            skillTree_->Open();
            skillTree_->Update();
        });
    }

    if (stage_->IsGameOver()) {
        state_ = OUTRO;
        HidePlayHud();
        PostEffect()->ApplyPreset("DarkScene", [&] {
            next_ = "gameover";
            Change();
        });
    }
}

void GameScene::StartNextRound() {
    gameTimer_->SetDuration(status_.time);
    stage_->Initialize();
    state_ = PLAY;

    // 2回目以降は中央の演出をせず、Objective とキルカウンターを最初から左上に出す
    stageInfoUI_->ShowAtCorner(status_.requirementKill);
    killCounter_->Setup(stage_->GetEnemies(), status_.requirementKill);
    killCounter_->SetActive(true);
    ShowPlayHud();
}

void GameScene::ShowPlayHud() {
    keyGuide_->SetActive(true);
    // タイマーはフェードインが終わってから時間が進み始める
    gameTimer_->PlayIntroAnim();
}

void GameScene::HidePlayHud() {
    stageInfoUI_->SetActive(false);
    gameTimer_->SetActive(false);
    keyGuide_->SetActive(false);
    killCounter_->SetActive(false);
}
