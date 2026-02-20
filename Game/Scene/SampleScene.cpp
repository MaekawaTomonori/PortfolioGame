#include "SampleScene.hpp"
#include "Framework.hpp"
#include "imgui.h"
#include "Random/RandomEngine.hpp"
#include "include/Pattern/Singleton.hpp"

void SampleScene::Initialize() {
    name_ = "sample";

    status_ = std::make_unique<GameStatus>();

    userInterface_ = std::make_unique<Ui::UserInterface>();
}

void SampleScene::Update() {
    userInterface_->Update();
}

void SampleScene::Draw() {
    userInterface_->Draw();
}

void SampleScene::Debug() {
    userInterface_->StandaloneDebug();
}
