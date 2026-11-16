#include "TitleScene.hpp"
#include "Input.hpp"
#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

TitleScene::~TitleScene() {
    ui_.Unload();
}

void TitleScene::Initialize() {
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

    KeyBinds binds;
    binds.Load();

    stage_ = std::make_unique<Stage>(status_);
    stage_->Setup(Particle(), PostEffect(), binds);
    stage_->Initialize();
    stage_->SetActive(false);

    ui_ = GEUI::Canvas::Load("Assets/UI/Title.json");
    start_ = ui_.Find("Start");
    quit_ = ui_.Find("Quit");
    start_.SetSelected(true);

    // ボタンはマウスでも選べるので、タイトルの間はカーソルを出す
    Singleton<Input>::GetInstance()->SetCursorVisible(true);
}

void TitleScene::Update() {
    HandleInput();
    stage_->Update();
}

void TitleScene::HandleInput() {
    const auto input = Singleton<Input>::GetInstance();
    if (input->IsTrigger(DIK_UP)   || input->IsTrigger(DIK_W)) ui_.Navigate(GEUI::Direction::Up);
    if (input->IsTrigger(DIK_DOWN) || input->IsTrigger(DIK_S)) ui_.Navigate(GEUI::Direction::Down);
    if (input->IsTrigger(DIK_RETURN) || input->IsTrigger(DIK_SPACE)) ui_.Submit();

    if (start_.WasPressed()) {
        // GEUI は暗転より手前に描かれるので、先に消してから暗転させる
        ui_.SetVisible(false);
        next_ = "game";
        Change();
    } else if (quit_.WasPressed()) {
        // Engine のメインループは WM_QUIT を受け取ると終わる
        PostQuitMessage(0);
    }
}

void TitleScene::Draw() {
    stage_->Draw();
}

void TitleScene::Debug() {
    stage_->Debug();
}

void TitleScene::Finalize() {
    // ゲーム中はカーソルを消しておく（メニューを開いたときだけ GameScene が出す）
    Singleton<Input>::GetInstance()->SetCursorVisible(false);
    IScene::Finalize();
}

void TitleScene::OnEnable() {
    Singleton<CameraDirector>::GetInstance()->Run("cp", true);
}
