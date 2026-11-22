#include "PlayHud.hpp"

#include "Skill/SkillManager.hpp"

void PlayHud::Initialize(const KeyBinds& _binds, const UiTheme& _theme) {
    gameTimer_ = std::make_unique<GameTimer>();
    gameTimer_->Initialize(_theme);

    keyGuide_ = std::make_unique<KeyGuide>();
    keyGuide_->Initialize(_binds, _theme);

    killCounter_ = std::make_unique<KillCounter>();
    killCounter_->Initialize(_theme);

    stageInfo_ = std::make_unique<StageInfoUI>();
    stageInfo_->Initialize(_theme);
}

void PlayHud::Update() {
    // どれも非表示の間は何もしないので、まとめて呼んでよい
    gameTimer_->Update();
    stageInfo_->Update();
    killCounter_->Update();
    keyGuide_->Update();
}

void PlayHud::Debug() {
    gameTimer_->Debug();
    keyGuide_->Debug();
    killCounter_->Debug();
}

void PlayHud::PlayIntro(const uint16_t _requirementKill, Enemies* _enemies, const float _duration) {
    gameTimer_->SetDuration(_duration);

    // 目標が左上へ動き始めたら、その下の撃破数をその場でフェードインさせる
    stageInfo_->SetOnMoveStart([this, _requirementKill, _enemies] {
        killCounter_->Setup(_enemies, _requirementKill);
        killCounter_->PlayIntroAnim();
    });
    stageInfo_->Setup(_requirementKill);
}

void PlayHud::StartRound(const uint16_t _requirementKill, Enemies* _enemies, const float _duration) {
    gameTimer_->SetDuration(_duration);

    stageInfo_->ShowAtCorner(_requirementKill);
    killCounter_->Setup(_enemies, _requirementKill);
    killCounter_->SetActive(true);
}

void PlayHud::Show() {
    keyGuide_->SetActive(true);
    // 残り時間はフェードインが終わってから進み始める
    gameTimer_->PlayIntroAnim();
}

void PlayHud::Hide() {
    stageInfo_->SetActive(false);
    gameTimer_->SetActive(false);
    keyGuide_->SetActive(false);
    killCounter_->SetActive(false);
}

void PlayHud::SetCooldowns(const SkillManager& _skillManager) {
    keyGuide_->SetCooldown(InputAction::Ability1, _skillManager.GetCooldownRatio(SkillSlot::Ability1));
    keyGuide_->SetCooldown(InputAction::Ability2, _skillManager.GetCooldownRatio(SkillSlot::Ability2));
}

bool PlayHud::IsIntroFinished() const {
    return stageInfo_->IsFinish();
}

bool PlayHud::IsTimeUp() const {
    return gameTimer_->IsDone();
}
