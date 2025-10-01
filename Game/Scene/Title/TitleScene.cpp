#include "TitleScene.hpp"

void TitleScene::Initialize() {
    skybox_ = std::make_unique<Skybox>();
    skybox_->Initialize("Skybox.dds");

    terrain_= std::make_unique<Model>();
    terrain_->Initialize("plane");
    terrain_->SetScale({ 100.f, 100.f, 1.f });
    terrain_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });
    terrain_->SetEnvironmentTexture("skybox.dds");
}

void TitleScene::Update() {
    skybox_->Update();
    terrain_->Update();
}

void TitleScene::Draw() {
    skybox_->Draw();
    terrain_->Draw();
}

void TitleScene::Finalize() {
    IScene::Finalize();
}
