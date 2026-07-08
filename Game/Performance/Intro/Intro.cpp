#include "Intro.hpp"

#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

void Intro::Initialize() {
    cameraDone_ = false;

    Singleton<CameraDirector>::GetInstance()->SetAnchor({0.f, 0.f, -5.f});
    Singleton<CameraDirector>::GetInstance()->Run("intro", false, true);
}

void Intro::Update() {
    if (cameraDone_) return;
    cameraDone_ = !Singleton<CameraDirector>::GetInstance()->IsPlaying();
}
