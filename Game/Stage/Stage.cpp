#include "Stage.hpp"

#include "Math/MathUtils.hpp"
#include <cmath>

#undef max

namespace {
    constexpr float DeltaTime      = 1.f / 60.f;
    constexpr float FOG_INTERVAL   = 0.5f;
}

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
    terrain_->SetName("Field");

    player_ = std::make_unique<Player>(particle_, postEffect_);
    enemies_ = std::make_unique<Enemies>(particle_, status_);

    skillManager_ = std::make_unique<SkillManager>();
    skillManager_->SetEnemies(enemies_.get());
    skillManager_->SetParticle(particle_);

    player_->SetOnSkillRequest([this](const Vector3& _pos, const Vector3& _dir) {
        skillManager_->SpawnBlackHole(_pos, _dir);
    });

    // フィールドの暗闇感を演出する霧エフェクトのスポーン関数（発生中心の周囲にランダム配置）
    particle_->RegisterSpawnFunc("field_fog_spawn", 
        [](const Vector3& center, Vector3& pos, Vector3& vel) {
            float angle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
            float radius = MathUtils::Random(0.5f, 10.5f);
            pos = {
                center.x + std::cos(angle) * radius,
                center.y + MathUtils::Random(-0.2f, 0.3f),
                center.z + std::sin(angle) * radius,
            };
            float horizAngle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
            float horizSpeed = MathUtils::Random(0.f, 0.2f);
            vel = {
                std::cos(horizAngle) * horizSpeed,
                MathUtils::Random(0.35f, 0.65f),
                std::sin(horizAngle) * horizSpeed,
            };
        }
    );

    // フィールド霧エフェクトの更新関数（上昇しながらフェードアウト）
    particle_->RegisterUpdateFunc("field_fog_update",
        [](float t, const Vector3&, Vector3&, Vector3& vel, Vector4& color) {
            vel.y *= 0.97f;
            vel.x *= 0.93f;
            vel.z *= 0.93f;
            float alpha = (t < 0.15f)
                ? (t / 0.15f) * 0.5f
                : 0.5f * (1.f - (t - 0.15f) / 0.85f);
            color.w = std::max(0.f, alpha);
            color.x = 0.32f;
            color.y = 0.28f;
            color.z = 0.42f;
        }
    );
}

void Stage::Initialize() {
    player_->Initialize();
    player_->SetStatus(status_.playerStatus);

    enemies_->Initialize();
    enemies_->SetTarget(player_.get());

    skillManager_->Initialize();
}

void Stage::Update() {
    if (active_ != pending_) {
        active_ = pending_;
    }

    skybox_->Update();
    terrain_->Update();

    if (!active_) {
        player_->UpdateWithoutInput();
        return;
    }

    enemies_->Update();

    if (!enemies_->Empty()) {
        player_->SetTargetPosition(enemies_->GetNearest(player_->GetPosition()));
    }

    player_->Update(DeltaTime);

    // フィールド霧エフェクトをプレイヤー位置を中心に継続発生
    fogTimer_ -= DeltaTime;
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
