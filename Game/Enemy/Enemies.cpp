#define NOMINMAX
#include "Enemies.hpp"

#include <algorithm>

#include "imgui.h"
#include "Command/Move/ToTargetCommand.hpp"
#include "Player/Movement/WalkBehavior.hpp"
#include "Player/Movement/DashBehavior.hpp"

void Enemies::Initialize(ParticleSystem* _particle) {
    timer_ = 0.f;
    particle_ = _particle;

    if (!particle_) Utils::Alert("ParticleSystem is null");

    // 共有Behaviorを生成（Flyweight Pattern）
    walkBehavior_ = std::make_unique<WalkBehavior>(3.0f);
    dashBehavior_ = std::make_unique<DashBehavior>(8.0f, 0.5f, 2.0f);

    // 共有Commandを生成
    toTargetCommand_ = std::make_unique<ToTargetCommand>(3.0f, 0.5f);

    particle_->Register("enemy_hit", { 3.f, 2.f, 0.f })
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
                    velocity = Vector3::Random() * 6.0f;
                }
                // 減速と重力
                velocity = velocity * 0.92f;
                velocity.y -= 0.05f;
                // フェードアウト
                color.w = 0.9f * (.7f - t);
            }
        });
#ifdef _DEBUG
    line_.Initialize();
    line_.SetColor({0.3f, 1.f, 0.3f, 1.f});
#endif
}


void Enemies::Update() {
    constexpr float DeltaTime = 1.f / 60.f;

    std::erase_if(enemies_, [](const auto& _enemy) {return !_enemy->IsActive();});

#ifdef _DEBUG

    line_.Clear();
    Vector3 p = target_->GetPosition();

    constexpr int segments = 32;
    constexpr float angleStep = 2.f * MathUtils::F_PI / static_cast<float>(segments);

    for (int i = 0; i < segments; ++i) {
        float ang1 = angleStep * i;
        float ang2 = angleStep * ((i + 1) % segments);

        Vector3 point1 = p + Vector3{
            cosf(ang1) * distance_.x,
            0.f,
            sinf(ang1)* distance_.x
        };
        Vector3 point2 = p + Vector3{
            cosf(ang2) * distance_.x,
            0.f,
            sinf(ang2)* distance_.x
        };
        line_.AddLine(point1, point2);
    }

    for (int i = 0; i < segments; ++i) {
        float ang1 = angleStep * i;
        float ang2 = angleStep * ((i + 1) % segments);

        Vector3 point1 = p + Vector3{
            cosf(ang1) * distance_.y,
            0.f,
            sinf(ang1)* distance_.y
        };
        Vector3 point2 = p + Vector3{
            cosf(ang2) * distance_.y,
            0.f,
            sinf(ang2)* distance_.y
        };

        line_.AddLine(point1, point2);
    }

    line_.Update();

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

#ifdef _DEBUG
    line_.Draw();
#endif
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
#ifdef _DEBUG
    ImGui::Begin("Enemies");

    ImGui::Text("Spawn Distance Range");

    ImGui::SetNextItemWidth(120.0f);
    bool minChanged = ImGui::DragFloat("##Min", &distance_.x, 0.1f, 0.0f, 24.9f, "%.1f");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    bool maxChanged = ImGui::DragFloat("##Max", &distance_.y, 0.1f, 0.1f, 25.0f, "%.1f");

    // Minを動かしたときの処理
    if (minChanged && !maxChanged) {
        distance_.x = std::clamp(distance_.x, 0.0f, 24.9f);
        if (distance_.x >= distance_.y) {
            distance_.y = std::min(distance_.x + 0.1f, 25.0f);
        }
    }

    // Maxを動かしたときの処理
    if (maxChanged && !minChanged) {
        distance_.y = std::clamp(distance_.y, 0.1f, 25.0f);
        if (distance_.y <= distance_.x) {
            distance_.x = std::max(distance_.y - 0.1f, 0.f);
        }
    }

    ImGui::Separator();

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

    ImGui::Separator();

    // Individual enemy details
    if (ImGui::CollapsingHeader("Enemy Details")) {
        int index = 0;
        for (const auto& enemy : enemies_) {
            ImGui::PushID(index);

            // Enemy header with status indicator
            bool isDying = enemy->IsDead();
            bool isActive = enemy->IsActive();

            ImVec4 headerColor = isActive ?
                (isDying ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f)) :
                ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

            ImGui::TextColored(headerColor, "Enemy #%d", index);
            ImGui::SameLine();

            const char* status = isActive ? (isDying ? "[Dying]" : "[Active]") : "[Inactive]";
            ImGui::Text("%s", status);

            // Enemy debug info
            enemy->Debug();

            ImGui::PopID();
            ++index;
        }
    }

    ImGui::End();
#endif
}
bool Enemies::Empty() const {
    return enemies_.empty();
}

void Enemies::Spawn() {
    if (!target_) return;
    if (MaxEnemies <= enemies_.size()) return;

    Vector3 position = target_->GetPosition();
    position += Vector3::Random().Normalize() * MathUtils::Random(distance_.x, distance_.y);
    position.y = 1.5f;

    std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetPosition(position);
    enemy->SetParticleSystem(particle_);
    enemy->SetTarget(target_);

    // 共有Commandを設定（ポインタのみ）
    enemy->SetMoveCommand(toTargetCommand_.get());

    // 共有Behaviorを設定（ポインタのみ）
    if (auto* movement = enemy->GetMovement()) {
        movement->ClearBehaviors();
        movement->AddBehavior(walkBehavior_.get());
    }

    enemies_.push_back(std::move(enemy));
}

float Enemies::GetFarthestEnemyDistance(Vector3 referencePos) const {
    float maxDistance = 0.0f;

    for (const auto& enemy : enemies_) {
        if (!enemy->IsActive() || enemy->IsDead()) continue;

        float dist = (enemy->GetPosition() - referencePos).Length();
        if (dist > maxDistance) {
            maxDistance = dist;
        }
    }

    return maxDistance;
}
