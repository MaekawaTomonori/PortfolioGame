#include "GameClear.hpp"

#include "Camera/Controller/CameraController.hpp"
#include "Light/LightManager.hpp"
#include "Pattern/Singleton.hpp"
#include "Player/Player.hpp"
#include "Random/RandomEngine.hpp"

namespace {
    // プレイヤーのキューブを、少し上から見下ろして展示する
    constexpr Vector3 CAMERA_POSITION = {0.f, 2.5f, -11.1f};
    constexpr Vector3 CAMERA_ROTATION = {0.2f, 0.f, 0.f};
    constexpr Vector3 LIGHT_POSITION = {0.f, 2.f, 0.f};

    constexpr float SHOWCASE_ROTATE_SPEED = 0.7f;
    constexpr float SHOWCASE_WAVE_AMPLITUDE = 0.5f;

    constexpr Vector2 MESSAGE_POSITION = {640.f, 190.f};
}

void GameClear::Initialize() {
    entryTransition_ = Transition::Type::Fade;

    std::unique_ptr<Model> model = std::make_unique<Model>();
    model->Initialize("animatedcube");
    model->SetTranslate({});
    model->SetTexture("white_x16.png");
    model->SetColor(Player::BASE_COLOR);

    Singleton<LightManager>::GetInstance()->SetPosition(LIGHT_POSITION);
    showcase_ = std::make_unique<ItemShowcase>(std::move(model));
    showcase_
        ->SetRotateSpeed(SHOWCASE_ROTATE_SPEED)
        .SetWaveAmplitude(SHOWCASE_WAVE_AMPLITUDE);
    showcase_->Update();

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("congrats.png");
    sprite_->SetPosition(MESSAGE_POSITION);
}

void GameClear::Update() {
    if (menuInput_.IsSubmit()) {
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
        CAMERA_ROTATION,
        CAMERA_POSITION
    };
}
