#define NOMINMAX
#include "GameOver.hpp"

#include "Input.hpp"
#include "Pattern/Singleton.hpp"
#include "PostProcess/Executor/PostProcessExecutor.hpp"

void GameOver::Initialize() {
    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("go.png");
    sprite_->SetPosition({ 640.f, 360.f });
    sprite_->SetActivePostEffect(true);
}

void GameOver::Update() {
    alpha_ += 0.01f;
    alpha_ = std::min(alpha_, 1.f);

    if (Singleton<Input>::GetInstance()->IsTrigger(DIK_SPACE)) {
        next_ = "title";
        Change();
    }

    sprite_->SetColor({ 1.f, 1.f, 1.f, alpha_ });
    sprite_->Update();
}

void GameOver::Draw() {
    sprite_->Draw();
}

void GameOver::OnEnable() {
    alpha_ = 0.f;
    PostEffect()->ApplyPreset("DarkScene");
}
