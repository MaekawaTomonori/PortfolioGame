#include "SampleScene.hpp"
#include "Framework.hpp"
#include "imgui.h"
#include "Random/RandomEngine.hpp"
#include "include/Pattern/Singleton.hpp"

void SampleScene::Initialize() {
    name_ = "sample";
    // 煙エフェクト（ゆっくり上昇して広がる白い煙）
    Particle()
        ->Register("smoke", { -3.f, 0.f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.3f,
            .duration = 3.0f,
            .spawnCount = 3,
            .size = {0.5f, 0.5f, 0.5f},
            .velocity = {0.f, 0.8f, 0.f},  // 上昇
            .color = { 0.8f, 0.8f, 0.8f, 0.5f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // 横に広がる
                if (t < 0.01f) {
                    auto* random = Singleton<RandomEngine>::GetInstance();
                    velocity.x = random->Get(-0.2f, 0.2f);
                    velocity.z = random->Get(-0.2f, 0.2f);
                }
                // フェードアウト
                color.w = 0.5f * (1.0f - t);
            }
        });

    // ヒットエフェクト1（金色の爆発 - 外側に弾ける）
    Particle()
        ->Register("hit_gold", { 3.f, 2.f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.5f,
            .duration = 1.0f,
            .spawnCount = 15,
            .size = {0.3f, 0.3f, 0.3f},
            .velocity = {0.f, 0.f, 0.f},
            .color = { 1.f, 0.8f, 0.2f, 0.9f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // ランダムな方向に爆発（初回のみ設定）
                if (t < 0.01f) {
                    velocity = Vector3::Random() * 5.0f;  // より速く爆発
                }
                // 減速と重力
                velocity = velocity * 0.92f;  // より強い減速
                velocity.y -= 0.05f;  // 重力を加える
                // フェードアウト
                color.w = 0.9f * (1.0f - t);
            }
            });

    // ヒットエフェクト2（青い光のパルス波 - 外側に広がる輪）
    Particle()
        ->Register("hit_blue", { 3.f, 2.f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.7f,
            .duration = 1.5f,
            .spawnCount = 12,
            .size = {0.3f, 0.3f, 0.3f},
            .velocity = {0.f, 0.f, 0.f},
            .color = { 0.2f, 0.6f, 1.f, 0.9f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // 初回：放射状に外側へ（円形のパルス波）
                if (t < 0.01f) {
                    auto* random = Singleton<RandomEngine>::GetInstance();
                    float angle = random->Get(0.0f, 6.28318f);  // 0-2π
                    float speed = random->Get(1.f, 4.f);  // 拡散速度
                    velocity.x = std::cosf(angle) * speed;
                    velocity.z = std::sinf(angle) * speed;
                    velocity.y = 1.5f;  // 上昇
                }
                // 徐々に加速しながら外側へ広がる
                velocity.x *= 1.05f;
                velocity.z *= 1.05f;
                // 色を白→青にグラデーション
                color.x = 0.2f + (1.0f - t) * 0.6f;  // 白っぽく
                color.y = 0.6f + (1.0f - t) * 0.3f;
                // フェードアウト（後半で急速に）
                color.w = 0.9f * (1.0f - t * t);
            }
        });

    // キラキラエフェクト（星のような小さな粒子 - 点滅しながら降下）
    Particle()
        ->Register("sparkle", { 0.f, 3.f, -2.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.2f,
            .duration = 2.5f,
            .spawnCount = 20,
            .size = {0.2f, 0.2f, 0.2f},
            .velocity = {0.f, -0.6f, 0.f},  // 降下
            .color = { 1.f, 1.f, 0.5f, 1.f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // 初回にランダムな横方向の速度を追加
                if (t < 0.01f) {
                    auto* random = Singleton<RandomEngine>::GetInstance();
                    velocity.x = random->Get(-0.4f, 0.4f);
                    velocity.z = random->Get(-0.4f, 0.4f);
                }
                // 点滅効果
                float blink = (std::sinf(t * 25.0f) + 1.0f) * 0.5f;
                color.w = blink * 0.8f;
            }
            });

    // ダッシュ煙エフェクト（地面に沿って後ろに流れる）
    Particle()
        ->Register("dash_smoke", { 0.f, 0.2f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.1f,
            .duration = 0.8f,
            .spawnCount = 5,
            .size = {0.4f, 0.2f, 0.4f},
            .velocity = {0.f, 0.3f, 4.5f},  // 後方に速く流れる
            .color = { 0.7f, 0.7f, 0.7f, 0.6f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // 横に広がりながら減速
                if (t < 0.01f) {
                    auto* random = Singleton<RandomEngine>::GetInstance();
                    velocity.x = random->Get(-0.6f, 0.6f);
                }
                velocity = velocity * 0.96f;  // 減速
                // フェードアウト
                color.w = 0.6f * (1.0f - t);
            }
            });

    // 着地時の衝撃波エフェクト（円形に広がる）
    Particle()
        ->Register("landing_impact", { 0.f, 0.1f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 1.0f,
            .duration = 0.5f,
            .spawnCount = 12,
            .size = {0.3f, 0.1f, 0.3f},
            .velocity = {0.f, 0.f, 0.f},
            .color = { 0.9f, 0.9f, 0.8f, 0.7f },
            .updateFunc = [](float t, Vector3& velocity, Vector4& color) {
                // 放射状に広がる（初回のみ設定）
                if (t < 0.01f) {
                    velocity = Vector3::Random();
                    velocity.y = 0.f; // 地面に平行
                    velocity = velocity * 3.5f;  // より速く広がる
                }
                // 急速に減速
                velocity = velocity * 0.88f;
                // フェードアウト（二乗でより速く）
                color.w = 0.7f * (1.0f - t * t);
            }
        }); 

    userInterface_ = std::make_unique<SkillTree>();
    userInterface_->Initialize();

    model_ = std::make_unique<Model>();
    model_->Initialize("animatedcube");
}

void SampleScene::Update() {
    userInterface_->Update();
    model_->Update();
}

void SampleScene::Draw() {
    model_->Draw();
    userInterface_->Draw();
}

void SampleScene::Debug() {
    ImGui::Begin("Particles");

    ImGui::Text("Particle Effects");
    ImGui::Separator();

    if (ImGui::Button("Smoke Effect (ToggleSwitch)")) {
        smoke_ = !smoke_;
        Particle()->Edit("smoke").SetEnable(smoke_);
    }

    if (ImGui::Button("Gold Hit")) {
        Particle()->Edit("hit_gold").Emit();
    }
    if (ImGui::Button("Blue Hit")) {
        Particle()->Edit("hit_blue").Emit();
    }
    if (ImGui::Button("Sparkle")) {
        Particle()->Edit("sparkle").Emit();
    }
    if (ImGui::Button("Dash Smoke")) {
        Particle()->Edit("dash_smoke").Emit();
    }
    if (ImGui::Button("Landing Impact")) {
        Particle()->Edit("landing_impact").Emit();
    }


    ImGui::End();

    userInterface_->Debug();
}
