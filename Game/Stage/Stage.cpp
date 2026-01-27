#include "Stage.hpp"

void Stage::Setup(ParticleSystem* _particle, PostProcessExecutor* _postEffect) {
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
    terrain_->SetScale({ 100.f, 100.f, 1.f });
    terrain_->SetTilingMultiply({ 10.f, 10.f });
    terrain_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });
    terrain_->SetEnvironmentTexture("rostock.dds");
    terrain_->SetTexture("tile.png");

    player_ = std::make_unique<Player>(particle_, postEffect_);
    enemies_ = std::make_unique<Enemies>(particle_);
}

void Stage::Initialize(const GameStatus& _status) { 
    player_->Initialize();
    player_->SetStatus(_status.playerStatus);

    enemies_->Initialize();
    enemies_->SetTarget(player_.get());
    enemies_->SetMaxCount(_status.maxEnemyCount);
    enemies_->SetInterval(_status.enemySpawnInterval);
}

void Stage::Update() {
    skybox_->Update();
    terrain_->Update();
    enemies_->Update();

    if (!enemies_->Empty()) {
        player_->SetTargetPosition(enemies_->GetNearest(player_->GetPosition()));
    }

    player_->Update(1.f / 60.f);
}

void Stage::Draw() const {
    skybox_->Draw();
    terrain_->Draw();
    player_->Draw();
    enemies_->Draw();
}

void Stage::Debug() const {
    player_->Debug();
    enemies_->Debug();
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
    return RequirementKills <= enemies_->GetDeathCount();
}
