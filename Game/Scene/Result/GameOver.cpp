#define NOMINMAX
#include "GameOver.hpp"

#include "PostProcess/Executor/PostProcessExecutor.hpp"

void GameOver::Initialize() {
    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("go.png");
    sprite_->SetPosition({ 640.f, 360.f });
    sprite_->SetActivePostEffect(true);

    //bg_ = std::make_unique<Sprite>();
    //bg_->Initialize("white_x16.png");
    //bg_->SetColor({0.f,0.f,0.f,1.f});
    //bg_->SetPosition({640.f, 360.f});
    //bg_->SetSize({1280.f, 720.f});

    alpha_ = 0.f;

    PostEffect()->ApplyPreset("DarkScene");
}

void GameOver::Update() {
    alpha_ += 0.01f;
    alpha_ = std::min(alpha_, 1.f);

    sprite_->Update();
    //bg_->Update();

    sprite_->SetColor({ 1.f, 1.f, 1.f, alpha_ });
}

void GameOver::Draw() {
    //bg_->Draw();
    sprite_->Draw();
}
