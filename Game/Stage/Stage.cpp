#include "Stage.hpp"

void Stage::Initialize() {
    skybox_ = std::make_unique<Skybox>();
    skybox_->Initialize("rostock.dds");

    terrain_ = std::make_unique<Model>();
    terrain_->Initialize("plane");
    terrain_->SetScale({ 100.f, 100.f, 1.f });
    terrain_->SetTilingMultiply({10.f, 10.f});
    terrain_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });
    terrain_->SetEnvironmentTexture("rostock.dds");
    terrain_->SetTexture("block.png");

    player_ = std::make_unique<Player>();
    player_->Initialize();

    enemy_ = std::make_unique<Enemy>();
    enemy_->Initialize();
    enemy_->SetTarget(player_.get());
}

void Stage::Update() {
    skybox_->Update();
    terrain_->Update();
    player_->Update(1.f / 60.f);

    enemy_->Update(1.f / 60.f);
}

void Stage::Draw() const {
    skybox_->Draw();
    terrain_->Draw();
    player_->Draw();
    enemy_->Draw();

}

Player* Stage::GetPlayer() const {
    return player_.get();
}
