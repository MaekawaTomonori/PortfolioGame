#include "GameClear.hpp"

#include "Input.hpp"
#include "Camera/Controller/CameraController.hpp"
#include "Pattern/Singleton.hpp"

void GameClear::Initialize() {
    Singleton<CameraController>::GetInstance()->GetActive()->transform_ = {
        {1,1,1},
        Vector3{0.2f, 0.f, 0.f},
        {0.f, 2.5f, -11.1f}
    };

    std::unique_ptr<Model> model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
    showcase_ = std::make_unique<ItemShowcase>(std::move(model_));
    showcase_->SetDebug(*DebugUI())
        .SetRotateSpeed(0.7f)
        .SetWaveAmplitude(0.5f);

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("congrats.png");
    sprite_->SetPosition({ 640.f, 190.f });
}

void GameClear::Update() {
    if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
        next_ = "game";
        Change();
        return;
    }

    showcase_->Update();
    sprite_->Update();
}

void GameClear::Draw() {
    showcase_->Draw();
    sprite_->Draw();
}
