#include "Intro.hpp"

#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

void Intro::Initialize() {
    
    time_ = 0.f;
    cameraDone_ = false;
    spriteDone_ = false;

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("intro.png");
    sprite_->SetPosition({640, 360});

    Singleton<CameraDirector>::GetInstance()->Run("intro");
}

void Intro::Update() {
    if (!cameraDone_) {
        auto camera = Singleton<CameraDirector>::GetInstance();
        cameraDone_ = !camera->IsPlaying();
        return;
    }

    if (spriteDone_)return; 

    time_ += 1.f / 60.f /* delta time or some time increment */;

    if (time_ >= duration_) {
        spriteDone_ = true;
    }

    sprite_->Update();
}

void Intro::Draw() {
    if (cameraDone_ && !spriteDone_){
        sprite_->Draw();
    }
}

bool Intro::IsFinish() const {
    return cameraDone_ && spriteDone_;
}
