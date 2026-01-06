#include "Outro.hpp"

#include "Camera/Director/CameraDirector.hpp"
#include "Pattern/Singleton.hpp"

void Outro::Run() {
    Singleton<CameraDirector>::GetInstance()->Run("outro", false);
}

bool Outro::IsFinish() const {
    return !Singleton<CameraDirector>::GetInstance()->IsPlaying();
}
