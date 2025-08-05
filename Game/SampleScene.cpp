#include "SampleScene.hpp"

void SampleScene::Initialize() {
    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("circle2.png");
    sprite_->SetPosition({100, 100});

    sky_ = std::make_unique<Skybox>();
    sky_->Initialize("rostock_laage_airport_4k.dds");

    model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
    model_->SetEnvironmentTexture("rostock_laage_airport_4k.dds");
}

void SampleScene::Update() {
    if (!sprite_) return;
    sprite_->Update();

    if (!model_)return;
    model_->Update();

    if (!sky_)return;
    sky_->Update();
}

void SampleScene::Draw() {
    if (!sky_)return;
    sky_->Draw();

    if (!model_) return;
    model_->Draw();

    if (!sprite_) return;
    //sprite_->Draw();
}

void SampleScene::Finalize() {
    IScene::Finalize();
}
