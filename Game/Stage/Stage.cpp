#include "Stage.hpp"

#include "Effect/PerParticleUpdate.hpp"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "Light/LightManager.hpp"
#include "src/Time/Time.hpp"

#undef max

namespace {
    constexpr float FOG_INTERVAL = 0.5f;

    constexpr float FOG_SPAWN_RADIUS_MIN     = 0.5f;
    constexpr float FOG_SPAWN_RADIUS_MAX     = 10.5f;
    constexpr float FOG_SPAWN_HEIGHT_MIN     = -0.2f;
    constexpr float FOG_SPAWN_HEIGHT_MAX     = 0.3f;
    constexpr float FOG_HORIZONTAL_SPEED_MAX = 0.2f;
    constexpr float FOG_RISE_SPEED_MIN       = 0.35f;
    constexpr float FOG_RISE_SPEED_MAX       = 0.65f;

    constexpr float FOG_VERTICAL_DRAG   = 0.97f;
    constexpr float FOG_HORIZONTAL_DRAG = 0.93f;
    constexpr float FOG_FADE_IN_DURATION = 0.15f;
    constexpr float FOG_MAX_ALPHA        = 0.5f;
    constexpr Vector3 FOG_COLOR = {0.32f, 0.28f, 0.42f};

    constexpr float HIT_EXPLOSION_INITIAL_BURST_THRESHOLD = 0.01f;
    constexpr float HIT_EXPLOSION_BURST_SPEED = 8.0f;
    constexpr float HIT_EXPLOSION_DRAG        = 0.92f;
    constexpr float HIT_EXPLOSION_GRAVITY     = 0.05f;
    constexpr float HIT_EXPLOSION_LIFETIME    = 0.7f;
    constexpr float HIT_EXPLOSION_ALPHA_SCALE = 0.9f;

    constexpr float TERRAIN_SIZE = 100.f;
    // 見た目の地形端より内側でプレイヤーの移動を止めるための余白
    constexpr float FIELD_BOUNDARY_MARGIN = 5.f;
    constexpr float FIELD_HALF_EXTENT = TERRAIN_SIZE * 0.5f - FIELD_BOUNDARY_MARGIN;
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
    terrain_->SetScale({ TERRAIN_SIZE, TERRAIN_SIZE, 1.f });
    terrain_->SetTilingMultiply({ 10.f, 10.f });
    terrain_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });
    terrain_->SetTexture("tile.png");
    terrain_->SetName("Field");

    player_ = std::make_unique<Player>(particle_, postEffect_);
    player_->SetFieldBounds(FIELD_HALF_EXTENT);
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
            float radius = MathUtils::Random(FOG_SPAWN_RADIUS_MIN, FOG_SPAWN_RADIUS_MAX);
            pos = {
                center.x + std::cos(angle) * radius,
                center.y + MathUtils::Random(FOG_SPAWN_HEIGHT_MIN, FOG_SPAWN_HEIGHT_MAX),
                center.z + std::sin(angle) * radius,
            };
            float horizAngle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
            float horizSpeed = MathUtils::Random(0.f, FOG_HORIZONTAL_SPEED_MAX);
            vel = {
                std::cos(horizAngle) * horizSpeed,
                MathUtils::Random(FOG_RISE_SPEED_MIN, FOG_RISE_SPEED_MAX),
                std::sin(horizAngle) * horizSpeed,
            };
        }
    );

    // フィールド霧エフェクトの更新関数（上昇しながらフェードアウト）
    particle_->RegisterUpdateFunc("field_fog_update",
        PerParticleUpdate([](float t, const Vector3&, Vector3&, Vector3& vel, Vector4& color) {
            vel.y *= FOG_VERTICAL_DRAG;
            vel.x *= FOG_HORIZONTAL_DRAG;
            vel.z *= FOG_HORIZONTAL_DRAG;
            float alpha = (t < FOG_FADE_IN_DURATION)
                ? (t / FOG_FADE_IN_DURATION) * FOG_MAX_ALPHA
                : FOG_MAX_ALPHA * (1.f - (t - FOG_FADE_IN_DURATION) / (1.f - FOG_FADE_IN_DURATION));
            color = { FOG_COLOR.x, FOG_COLOR.y, FOG_COLOR.z, std::max(0.f, alpha)};
        })
    );

    // プレイヤー被弾時のヒットエフェクトの更新関数（弾け飛んで減衰しながらフェードアウト）
    particle_->RegisterUpdateFunc("hit_explosion", PerParticleUpdate([](float t, const Vector3&, Vector3& pos, Vector3& velocity, Vector4& color) {
        (void)pos;
        if (t < HIT_EXPLOSION_INITIAL_BURST_THRESHOLD) {
            velocity = Vector3::Random() * HIT_EXPLOSION_BURST_SPEED;
        }
        velocity = velocity * HIT_EXPLOSION_DRAG;
        velocity.y -= HIT_EXPLOSION_GRAVITY;
        color.w = HIT_EXPLOSION_ALPHA_SCALE * (HIT_EXPLOSION_LIFETIME - t);
    }));

    particle_->Register("hit");
}

void Stage::Initialize() {
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

    if (!enemies_->Empty()) {
        player_->SetTargetPosition(enemies_->GetNearest(player_->GetPosition()));
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
