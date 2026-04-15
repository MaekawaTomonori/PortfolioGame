#define NOMINMAX
#include "Enemies.hpp"

#include <algorithm>

#include "imgui.h"
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"

Enemies::Enemies(ParticleSystem* _particle, const GameStatus& _status) :status_(_status) {
    if (!_particle){Utils::Alert("ParticleSystem is null");}

    particle_ = _particle;

    // パラメータの読み込み
    LoadParams();

    particle_->RegisterUpdateFunc("enemy_hit_explosion", [](float t, const Vector3&, Vector3& pos, Vector3& velocity, Vector4& color) {
        (void)pos;
        if (t < 0.01f) {
            velocity = Vector3::Random() * 6.0f;
        }
        velocity = velocity * 0.92f;
        velocity.y -= 0.05f;
        color.w = 0.9f * (.7f - t);
    });

#ifdef _DEBUG
    line_.Initialize();
    line_.SetName("Enemies");
    line_.SetColor({0.3f, 1.f, 0.3f, 1.f});
#endif
}

void Enemies::Initialize() {
    timer_ = 0.f;
    deathCount_ = 0;
    done_ = false;
    enemies_.clear();
}

void Enemies::Update() {
    constexpr float DeltaTime = 1.f / 60.f;

    uint16_t killed = 0;
    for (const auto& enemy : enemies_) {
        if (!enemy->IsActive() && enemy->IsKilledByBullet()) {
            ++killed;
        }
    }
    std::erase_if(enemies_, [](const auto& _enemy) {return !_enemy->IsActive();});
    deathCount_ += killed;

    if (!done_ && status_.requirementKill > 0 && deathCount_ >= status_.requirementKill) {
        done_ = true;
        for (auto& enemy : enemies_) {
            enemy->ForceDeath();
        }
    }

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
        if (status_.enemySpawnInterval <= timer_) {
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

Vector3 Enemies::GetNearest(const Vector3& _pos) const {
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

    // Display current enemy count
    ImGui::Text("Active Enemies: %zu / %hu", enemies_.size(), status_.maxEnemyCount);

    ImGui::Separator();

    // Auto Spawn toggle with clear visual state
    ImGui::Checkbox("Auto Spawn", &autoSpawn_);
    ImGui::SameLine();
    ImGui::TextColored(autoSpawn_ ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),autoSpawn_ ? "[ON]" : "[OFF]");

    // Show spawn timer when auto spawn is enabled
    if (autoSpawn_) {
        ImGui::Text("Next spawn in: %.1fs", status_.enemySpawnInterval - timer_);
        ImGui::ProgressBar(timer_ / status_.enemySpawnInterval, ImVec2(-1.0f, 0.0f));
    }

    // Manual spawn button
    if (ImGui::Button("Spawn Enemy")) {
        Spawn();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Enemy Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        constexpr float inputWidth = 150.0f;

        // Spawn
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Spawn");
        ImGui::Text("Distance Range");
        ImGui::SetNextItemWidth(inputWidth * 0.5f);
        bool minChanged = ImGui::DragFloat("##Min", &distance_.x, 0.1f, 0.0f, 24.9f, "Min: %.1f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth * 0.5f);
        bool maxChanged = ImGui::DragFloat("##Max", &distance_.y, 0.1f, 0.1f, 25.0f, "Max: %.1f");

        if (minChanged && !maxChanged) {
            distance_.x = std::clamp(distance_.x, 0.0f, 24.9f);
            if (distance_.x >= distance_.y) distance_.y = std::min(distance_.x + 0.1f, 25.0f);
        }
        if (maxChanged && !minChanged) {
            distance_.y = std::clamp(distance_.y, 0.1f, 25.0f);
            if (distance_.y <= distance_.x) distance_.x = std::max(distance_.y - 0.1f, 0.f);
        }

        ImGui::Separator();

        // Stats
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Stats");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Max HP",       &enemyParams_.maxHp,       0.1f, 1.0f,  10.0f, "%.1f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Move Speed",   &enemyParams_.moveSpeed,   0.1f, 0.5f,  20.0f, "%.1f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Min Distance", &enemyParams_.minDistance, 0.1f, 0.0f,   5.0f, "%.2f");

        ImGui::Separator();

        // Combat
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Combat");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Invincible Duration", &enemyParams_.invincibleDuration, 0.05f, 0.1f, 2.0f, "%.2fs");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Knockback Decay", &enemyParams_.knockbackDecay, 0.05f, 0.1f, 1.0f, "%.2f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Death Duration", &enemyParams_.deathDuration, 0.05f, 0.1f, 3.0f, "%.2fs");

        ImGui::Separator();

        // Dash
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Dash");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Charge Time", &enemyParams_.dash.chargeMaxTime, 0.1f, 0.1f, 5.0f, "%.2fs");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Dash Duration", &enemyParams_.dash.duration, 0.05f, 0.1f, 2.0f, "%.2fs");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Dash Speed", &enemyParams_.dash.speed, 0.5f, 5.0f, 50.0f, "%.1f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Trigger Interval", &enemyParams_.dash.triggerInterval, 0.1f, 0.5f, 10.0f, "%.2fs");
        ImGui::SetNextItemWidth(inputWidth);
        float chancePercent = enemyParams_.dash.triggerChance * 100.0f;
        if (ImGui::SliderFloat("Trigger Chance", &chancePercent, 0.0f, 100.0f, "%.0f%%")) {
            enemyParams_.dash.triggerChance = chancePercent / 100.0f;
        }

        ImGui::Spacing();
        ImGui::Text("Prediction Line");
        ImGui::Indent();
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Enemy Offset", &enemyParams_.dash.enemyOffset, 0.1f, 0.0f, 5.0f, "%.2f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Line Length", &enemyParams_.dash.lineLength, 0.05f, 0.1f, 2.0f, "%.2f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Line Width", &enemyParams_.dash.lineWidth, 0.1f, 0.5f, 5.0f, "%.2f");
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::DragFloat("Center Offset", &enemyParams_.dash.lineCenterOffset, 0.05f, 0.0f, 1.0f, "%.2f");
        ImGui::Unindent();

        ImGui::Separator();

        if (ImGui::Button("Save to JSON", ImVec2(-1, 0))) SaveParams();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Assets/Data/EnemyParams.json");
    }

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

bool Enemies::IsDone() const {
    return done_ && enemies_.empty();
}

bool Enemies::Empty() const {
    return enemies_.empty();
}

void Enemies::Spawn() {
    if (done_) return;
    if (!target_) return;
    if (status_.maxEnemyCount <= enemies_.size()) return;

    Vector3 position = target_->GetPosition();
    position += Vector3::Random().Normalize() * MathUtils::Random(distance_.x, distance_.y);
    position.y = .5f;

    std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetPosition(position);
    enemy->SetScale({SIZE, SIZE, SIZE});
    enemy->SetParticleSystem(particle_);
    enemy->SetTarget(target_);

    // 共通パラメータへのポインタを設定（すべてのEnemyが同じパラメータを参照）
    enemy->SetParams(&enemyParams_);

    enemies_.push_back(std::move(enemy));
}

float Enemies::GetFarthestEnemyDistance(const Vector3& referencePos) const {
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

uint16_t Enemies::GetDeathCount() const {
    return deathCount_;
}

void Enemies::LoadParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();

    if (json->Load("EnemyParams")) {
        // 基本ステータス
        enemyParams_.maxHp      = std::get<float>(json->GetValue("EnemyParams", "Basic", "MaxHp"));
        enemyParams_.moveSpeed  = std::get<float>(json->GetValue("EnemyParams", "Basic", "MoveSpeed"));
        enemyParams_.minDistance= std::get<float>(json->GetValue("EnemyParams", "Basic", "MinDistance"));

        distance_ = std::get<Vector2>(json->GetValue("EnemyParams", "Basic", "SpawnDistance"));

        // 無敵時間
        enemyParams_.invincibleDuration = std::get<float>(json->GetValue("EnemyParams", "Invincible", "Duration"));

        // ノックバック
        enemyParams_.knockbackDecay = std::get<float>(json->GetValue("EnemyParams", "Knockback", "Decay"));

        // 死亡演出
        enemyParams_.deathDuration = std::get<float>(json->GetValue("EnemyParams", "Death", "Duration"));

        // ダッシュアクション
        enemyParams_.dash.chargeMaxTime = std::get<float>(json->GetValue("EnemyParams", "Dash", "ChargeMaxTime"));
        enemyParams_.dash.duration = std::get<float>(json->GetValue("EnemyParams", "Dash", "Duration"));
        enemyParams_.dash.speed = std::get<float>(json->GetValue("EnemyParams", "Dash", "Speed"));
        enemyParams_.dash.triggerInterval = std::get<float>(json->GetValue("EnemyParams", "Dash", "TriggerInterval"));
        enemyParams_.dash.triggerChance = std::get<float>(json->GetValue("EnemyParams", "Dash", "TriggerChance"));
        enemyParams_.dash.enemyOffset = std::get<float>(json->GetValue("EnemyParams", "Dash", "EnemyOffset"));
        enemyParams_.dash.lineLength = std::get<float>(json->GetValue("EnemyParams", "Dash", "LineLength"));
        enemyParams_.dash.lineWidth = std::get<float>(json->GetValue("EnemyParams", "Dash", "LineWidth"));
        enemyParams_.dash.lineCenterOffset = std::get<float>(json->GetValue("EnemyParams", "Dash", "LineCenterOffset"));
    }
    // JSONファイルが存在しない場合はデフォルト値を使用
}

void Enemies::SaveParams() {
    const auto& json = Singleton<JsonParams>::GetInstance();

    // 基本ステータス
    json->SetValue("EnemyParams", "Basic", "MaxHp",       enemyParams_.maxHp);
    json->SetValue("EnemyParams", "Basic", "MoveSpeed",   enemyParams_.moveSpeed);
    json->SetValue("EnemyParams", "Basic", "MinDistance", enemyParams_.minDistance);

    json->SetValue("EnemyParams", "Basic", "SpawnDistance", distance_);

    // 無敵時間
    json->SetValue("EnemyParams", "Invincible", "Duration", enemyParams_.invincibleDuration);

    // ノックバック
    json->SetValue("EnemyParams", "Knockback", "Decay", enemyParams_.knockbackDecay);

    // 死亡演出
    json->SetValue("EnemyParams", "Death", "Duration", enemyParams_.deathDuration);

    // ダッシュアクション
    json->SetValue("EnemyParams", "Dash", "ChargeMaxTime", enemyParams_.dash.chargeMaxTime);
    json->SetValue("EnemyParams", "Dash", "Duration", enemyParams_.dash.duration);
    json->SetValue("EnemyParams", "Dash", "Speed", enemyParams_.dash.speed);
    json->SetValue("EnemyParams", "Dash", "TriggerInterval", enemyParams_.dash.triggerInterval);
    json->SetValue("EnemyParams", "Dash", "TriggerChance", enemyParams_.dash.triggerChance);
    json->SetValue("EnemyParams", "Dash", "EnemyOffset", enemyParams_.dash.enemyOffset);
    json->SetValue("EnemyParams", "Dash", "LineLength", enemyParams_.dash.lineLength);
    json->SetValue("EnemyParams", "Dash", "LineWidth", enemyParams_.dash.lineWidth);
    json->SetValue("EnemyParams", "Dash", "LineCenterOffset", enemyParams_.dash.lineCenterOffset);

    json->Save("EnemyParams");
}
