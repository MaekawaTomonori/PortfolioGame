#include "Enemies.hpp"

#include "imgui.h"
#include "Command/Move/ToTargetCommand.hpp"

void Enemies::Initialize(ParticleSystem* _particle) {
    timer_ = 0.f;
    particle_ = _particle;

    if (!particle_) Utils::Alert("ParticleSystem is null");

    particle_->Register("hit", { 3.f, 2.f, 0.f })
        .AddEmitter({
            .texture = "white_x16.png",
            .active = false,
            .frequency = 0.5f,
            .duration = 0.7f,
            .spawnCount = 15,
            .size = {0.3f, 0.3f, 0.3f},
            .velocity = {0.f, 0.f, 0.f},
            .color = { 1.f, 0.2f, 0.2f, 0.9f },
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
}


void Enemies::Update() {
    constexpr float DeltaTime = 1.f / 60.f;

    std::erase_if(enemies_, [](const auto& _enemy) {return !_enemy->IsActive();});

#ifdef _DEBUG
    if (autoSpawn_) {
#endif
        if (Interval <= timer_) {
            timer_ = 0.f;
            Spawn();
        } else {
            timer_ += DeltaTime;
        }

#ifdef _DEBUG
    }
#endif

    for (const auto& enemy : enemies_) {
        enemy->Update(DeltaTime);
    }
}

void Enemies::Draw() const {
    for (const auto& enemy : enemies_) {
        enemy->Draw();
    }
}

Vector3 Enemies::GetNearest(const Vector3 _pos) const {
    float minDistance = FLT_MAX;
    Vector3 nearest = {};
    for (const auto& enemy : enemies_) {
        if (enemy->IsDead()) continue;
        Vector3 toEnemy = enemy->GetPosition() - _pos;
        if (toEnemy.Length() < minDistance) {
            minDistance = toEnemy.Length();
            nearest = enemy->GetPosition();
        }
    }
    return nearest;
}

void Enemies::SetTarget(GameObject* _target) {
    target_ = _target;
}

void Enemies::Debug() {
    ImGui::Begin("Enemies");

    // Auto Spawn toggle with clear visual state
    ImGui::Checkbox("Auto Spawn", &autoSpawn_);
    ImGui::SameLine();
    ImGui::TextColored(autoSpawn_ ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),autoSpawn_ ? "[ON]" : "[OFF]");

    // Show spawn timer when auto spawn is enabled
    if (autoSpawn_) {
        ImGui::Text("Next spawn in: %.1fs", Interval - timer_);
        ImGui::ProgressBar(timer_ / Interval, ImVec2(-1.0f, 0.0f));
    }

    ImGui::Separator();

    // Manual spawn button
    if (ImGui::Button("Spawn Enemy")) {
        Spawn();
    }

    // Display current enemy count
    ImGui::Text("Active Enemies: %zu / %hu", enemies_.size(), MaxEnemies);

    ImGui::End();
}

void Enemies::Spawn() {
    if (MaxEnemies <= enemies_.size()) return;

    std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetParticleSystem(particle_);
    enemy->SetTarget(target_);
    enemy->SetMoveCommand(std::make_unique<ToTargetCommand>());
    enemies_.push_back(std::move(enemy));
}
