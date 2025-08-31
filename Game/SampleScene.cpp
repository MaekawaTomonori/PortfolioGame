#include "SampleScene.hpp"
#include "Framework.hpp"
#include "Pattern/Singleton.hpp"

void SampleScene::Initialize() {
    //sprite_ = std::make_unique<Sprite>();
    //sprite_->Initialize("circle2.png");
    //sprite_->SetPosition({100, 100});

    sky_ = std::make_unique<Skybox>();
    sky_->Initialize("Skybox.dds");

    //model_ = std::make_unique<Model>();
    //model_->Initialize("animatedcube");
    //model_->SetEnvironmentTexture("Skybox.dds");
    
    // Player初期化
    player_ = std::make_unique<Player>();
    player_->Initialize();
    player_->SetPosition({0.0f, 0.0f, 0.0f});
}

void SampleScene::Update() {
    
    //if (!sprite_) return;
    //sprite_->Update();

    //if (!model_)return;
    //model_->Update();

    if (!sky_)return;
    sky_->Update();
    
    if (player_) {
        player_->Update(1.f/60.f);
    }
}

void SampleScene::Draw() {
    if (!sky_)return;
    sky_->Draw();

    //if (!model_) return;
    //model_->Draw();
    
    if (player_) {
        player_->Draw();
    }

    //if (!sprite_) return;
    //sprite_->Draw();
}

void SampleScene::Finalize() {
    IScene::Finalize();
}
