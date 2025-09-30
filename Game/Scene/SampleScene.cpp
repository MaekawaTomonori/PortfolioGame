#include "SampleScene.hpp"
#include "Framework.hpp"

void SampleScene::Initialize() {
    //sprite_ = std::make_unique<Sprite>();
    //sprite_->Initialize("circle2.png");
    //sprite_->SetPosition({100, 100});

    sky_ = std::make_unique<Skybox>();
    sky_->Initialize("Skybox.dds");

    //model_ = std::make_unique<Model>();
    //model_->Initialize("animatedcube");
    //model_->SetEnvironmentTexture("Skybox.dds");

    plane_ = std::make_unique<Model>();
    plane_->Initialize("plane");
    plane_->SetScale({100.f, 100.f, 1.f});
    plane_->SetRotate({ -(MathUtils::F_PI/2.f), 0.f, 0.f });
    plane_->SetEnvironmentTexture("skybox.dds");

    // Player初期化
    player_ = std::make_unique<Player>();
    player_->Initialize();
    player_->SetPosition({0.0f, 0.0f, 0.0f});
    
    // CameraController初期化
    cameraController_ = std::make_unique<FollowCamera>();
    cameraController_->Initialize();
    cameraController_->SetTarget(player_.get());

    enemy_ = std::make_unique<Enemy>();
    enemy_->Initialize();
    enemy_->SetTarget(player_.get());
}

void SampleScene::Update() {
    
    //if (!sprite_) return;
    //sprite_->Update();

    //if (!model_)return;
    //model_->Update();

    if (!sky_)return;
    sky_->Update();

    if (plane_) {
        plane_->Update();
    }

    if (player_) {
        player_->Update(1.f/60.f);
    }

    if (enemy_) {
        enemy_->Update(1.f / 60.f);
    }
    
    if (cameraController_) {
        cameraController_->Update(1.f/60.f);
    }
}

void SampleScene::Draw() {
    if (!sky_)return;
    sky_->Draw();

    if (plane_) {
        plane_->Draw();
    }

    //if (!model_) return;
    //model_->Draw();
    
    if (player_) {
        player_->Draw();
    }

    if (enemy_) {
        enemy_->Draw();
    }

    //if (!sprite_) return;
    //sprite_->Draw();
}

void SampleScene::Finalize() {
    IScene::Finalize();
}
