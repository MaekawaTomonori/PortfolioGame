#include "TitleScene.hpp"
#include "Input.hpp"
#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

TitleScene::~TitleScene() {
    ui_.Unload();
}

void TitleScene::Initialize() {
    exitTransition_ = Transition::Type::Fade;

    // 背景のデモでは、敵を1体だけ出し続ける
    status_.requirementKill = 10;
    status_.maxEnemyCount = 1;
    status_.enemySpawnInterval = 0.f;

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
    if (const auto direction = menuInput_.GetDirection()) ui_.Navigate(*direction);
    if (menuInput_.IsSubmit()) ui_.Submit();

    if (start_.WasPressed()) {
        // GEUI は暗転より手前に描かれるので、先に消してから暗転させる
        ui_.SetVisible(false);
        next_ = "game";
        Change();
    } else if (quit_.WasPressed()) {
        Quit();
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
