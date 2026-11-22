#include "Stage.hpp"

#include "Effect/ParticleEffects.hpp"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "Light/LightManager.hpp"
#include "Skill/BasicShot/BasicShotSkill.hpp"
#include "Skill/BlackHole/BlackHoleSkill.hpp"
#include "Skill/Flash/FlashSkill.hpp"
#include "src/Time/Time.hpp"

namespace {
    constexpr float FOG_INTERVAL = 0.5f;

    constexpr float TERRAIN_SIZE = 100.f;
    // 見た目の地形端より内側でプレイヤーの移動を止めるための余白
    constexpr float FIELD_BOUNDARY_MARGIN = 5.f;
    constexpr float FIELD_HALF_EXTENT = TERRAIN_SIZE * 0.5f - FIELD_BOUNDARY_MARGIN;
}

void Stage::Setup(const GESTD::ReferencePtr<ParticleSystem>& _particle, const GESTD::ReferencePtr<PostProcessExecutor>& _postEffect, const KeyBinds& _binds) {
    // 初回限定
    particle_ = _particle;
    postEffect_ = _postEffect;
    if (!particle_) Utils::Alert("ParticleSystem is null");
    if (!postEffect_) Utils::Alert("PostProcessExecutor is null");

    skybox_ = std::make_unique<Skybox>();
    skybox_->Initialize("rostock.dds");
    skybox_->SetColor({0.f, 0.f, 0.f, 1.f});

    terrain_ = std::make_unique<Model>();
    terrain_->Initialize("plane");
    terrain_->SetScale({ TERRAIN_SIZE, TERRAIN_SIZE, 1.f });
    terrain_->SetTilingMultiply({ 10.f, 10.f });
    terrain_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });
    terrain_->SetTexture("tile.png");
    terrain_->SetName("Field");

    player_ = std::make_unique<Player>(particle_, postEffect_, _binds);
    player_->SetFieldBounds(FIELD_HALF_EXTENT);
    enemies_ = std::make_unique<Enemies>(particle_, status_);

    skillManager_ = std::make_unique<SkillManager>();
    skillManager_->Equip(SkillSlot::AutoAttack, std::make_unique<BasicShotSkill>(status_.playerStatus));
    skillManager_->Equip(SkillSlot::Ability1, std::make_unique<BlackHoleSkill>(enemies_.get(), particle_));
    skillManager_->Equip(SkillSlot::Ability2, std::make_unique<FlashSkill>());

    player_->SetOnSkillRequest([this](SkillSlot _slot, const Vector3& _pos, const Vector3& _dir) {
        skillManager_->Activate(_slot, {*player_, _pos, _dir });
    });

    if (!particle_) return;

    ParticleEffects::RegisterFieldFog(*particle_);
    ParticleEffects::RegisterHitExplosion(*particle_);
    particle_->Register("hit");
}

void Stage::Initialize() const {
    player_->Initialize();
    player_->SetStatus(status_.playerStatus);

    // プレイヤーの初期位置にライトを追従させる
    Singleton<LightManager>::GetInstance()->SetPosition(player_->GetLightAnchor());

    enemies_->Initialize();
    enemies_->SetTarget(player_.get());

    skillManager_->Initialize();
}

void Stage::Update() {
    if (active_ != pending_) {
        active_ = pending_;
        player_->SetEnable(active_);
    }

    skybox_->Update();
    terrain_->Update();

    if (!active_) {
        player_->UpdateWithoutInput();
        // 非アクティブ時もプレイヤー位置にライトを追従させる
        Singleton<LightManager>::GetInstance()->SetPosition(player_->GetLightAnchor());
        return;
    }

    enemies_->Update();

    if (auto nearest = enemies_->GetNearest(player_->GetPosition())) {
        player_->SetTargetPosition(*nearest);
    }

    player_->Update();

    // プレイヤー位置にライトを追従させる
    Singleton<LightManager>::GetInstance()->SetPosition(player_->GetLightAnchor());

    // フィールド霧エフェクトをプレイヤー位置を中心に継続発生
    fogTimer_ -= Time().GetDeltaTime();
    if (fogTimer_ <= 0.f) {
        particle_->Emit("field", player_->GetPosition());
        fogTimer_ = FOG_INTERVAL;
    }

    skillManager_->Update();
}

void Stage::Draw() const {
    skybox_->Draw();
    terrain_->Draw();
    player_->Draw();
    enemies_->Draw();
    skillManager_->Draw();
}

void Stage::Debug() {
    player_->Debug();
    enemies_->Debug();
    skillManager_->Debug();
}

Player* Stage::GetPlayer() const {
    return player_.get();
}

Enemies* Stage::GetEnemies() const {
    return enemies_.get();
}

void Stage::SetCamera(FollowCamera* _camera) const {
    player_->SetCamera(_camera);
}

bool Stage::IsClear() const {
    return enemies_->IsDone();
}

bool Stage::IsGameOver() const {
    return player_->IsDead();
}
