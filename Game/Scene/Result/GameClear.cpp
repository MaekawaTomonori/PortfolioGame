#include "GameClear.hpp"

#include "Input.hpp"
#include "imgui_internal.h"
#include "Camera/Controller/CameraController.hpp"
#include "Light/LightManager.hpp"
#include "Pattern/Singleton.hpp"
#include "Random/RandomEngine.hpp"

void GameClear::Initialize() {
    entryTransition_ = Transition::Type::Fade;

    std::unique_ptr<Model> model = std::make_unique<Model>();
    model->Initialize("animatedcube");
    model->SetTranslate({});
    model->SetTexture("white_x16.png");
    model->SetColor({ 0.3f, 0.3f, 1.f, 1.f });

    Singleton<LightManager>::GetInstance()->SetPosition({0.f,2.f,0.f});
    showcase_ = std::make_unique<ItemShowcase>(std::move(model));
    showcase_
        ->SetRotateSpeed(0.7f)
        .SetWaveAmplitude(0.5f);
    showcase_->Update();

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("congrats.png");
    sprite_->SetPosition({ 640.f, 190.f });
}

void GameClear::Update() {
    if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
        next_ = "title";
        Change();
        return;
    }

    showcase_->Update();
}

void GameClear::Draw() {
    showcase_->Draw();
    sprite_->Draw();
}

void GameClear::Debug() {

}

void GameClear::OnEnable() {
    Singleton<CameraController>::GetInstance()->GetActive()->transform_ = {
        {1,1,1},
        Vector3{0.2f, 0.f, 0.f},
        {0.f, 2.5f, -11.1f}
    };
}
