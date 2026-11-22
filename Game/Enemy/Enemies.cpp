#include "Enemies.hpp"

#include <algorithm>

#include "Utils.hpp"
#include "Effect/ParticleEffects.hpp"
#include "imgui.h"
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Time/Time.hpp"

Enemies::Enemies(ParticleSystem* _particle, const GameStatus& _status) :status_(_status) {
    particle_ = _particle;

    // パラメータの読み込み
    LoadParams();

#ifdef _DEBUG
    line_.Initialize();
    line_.SetName("Enemies");
    line_.SetColor({0.3f, 1.f, 0.3f, 1.f});
#endif

    // Alert は止まらないので、パーティクルの登録だけ飛ばす（Enemy 側は particle_ が null でも動く）
    if (!particle_) {
        Utils::Alert("ParticleSystem is null");
        return;
    }

    ParticleEffects::RegisterEnemyHitExplosion(*particle_);
}

void Enemies::Initialize() {
    timer_ = 0.f;
    deathCount_ = 0;
    done_ = false;
    enemies_.clear();
}

void Enemies::Update() {
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

    DrawSpawnRange();

    if (autoSpawn_) {
        if (status_.enemySpawnInterval <= timer_) {
            timer_ = 0.f;
            Spawn();
        } else {
            timer_ += Time().GetDeltaTime();
        }
    }

    for (const auto& enemy : enemies_) {
        enemy->Update();
    }
}

void Enemies::DrawSpawnRange() {
#ifdef _DEBUG
    if (!target_) return;

    constexpr int CIRCLE_SEGMENTS = 32;
    constexpr float ANGLE_STEP = 2.f * MathUtils::F_PI / static_cast<float>(CIRCLE_SEGMENTS);

    const Vector3 center = target_->GetPosition();
    const auto addCircle = [&](const float _radius) {
        for (int i = 0; i < CIRCLE_SEGMENTS; ++i) {
            const float angle = ANGLE_STEP * static_cast<float>(i);
            const float nextAngle = ANGLE_STEP * static_cast<float>(i + 1);
            line_.AddLine(
                center + Vector3{std::cos(angle) * _radius, 0.f, std::sin(angle) * _radius},
                center + Vector3{std::cos(nextAngle) * _radius, 0.f, std::sin(nextAngle) * _radius});
        }
    };

    line_.Clear();
    addCircle(distance_.x);
    addCircle(distance_.y);
    line_.Update();
#endif
}

void Enemies::Draw() const {
    for (const auto& enemy : enemies_) {
        enemy->Draw();
    }

#ifdef _DEBUG
    line_.Draw();
#endif
}

std::optional<Vector3> Enemies::GetNearest(const Vector3& _pos) const {
    if (enemies_.empty()) return std::nullopt;

    float minDistance = FLT_MAX;
    std::optional<Vector3> nearest = std::nullopt;
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

    ImGui::Text("Active Enemies: %zu / %hu", enemies_.size(), status_.maxEnemyCount);

    ImGui::Separator();

    ImGui::Checkbox("Auto Spawn", &autoSpawn_);
    if (autoSpawn_) {
        ImGui::Text("Next spawn in: %.1fs", status_.enemySpawnInterval - timer_);
        ImGui::ProgressBar(timer_ / status_.enemySpawnInterval, ImVec2(-1.0f, 0.0f));
    }

    if (ImGui::Button("Spawn Enemy")) {
        Spawn();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Enemy Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        constexpr float INPUT_WIDTH = 150.0f;

        // Spawn
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Spawn");
        ImGui::Text("Distance Range");
        ImGui::SetNextItemWidth(INPUT_WIDTH * 0.5f);
        bool minChanged = ImGui::DragFloat("##Min", &distance_.x, 0.1f, 0.0f, 24.9f, "Min: %.1f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(INPUT_WIDTH * 0.5f);
        bool maxChanged = ImGui::DragFloat("##Max", &distance_.y, 0.1f, 0.1f, 25.0f, "Max: %.1f");

        // 内側が外側を追い越さないように、動かしていない方を押し出す
        if (minChanged && distance_.x >= distance_.y) distance_.y = std::min(distance_.x + 0.1f, 25.0f);
        if (maxChanged && distance_.y <= distance_.x) distance_.x = std::max(distance_.y - 0.1f, 0.f);

        ImGui::Separator();

        // Stats
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Stats");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Max HP",       &enemyParams_.maxHp,       0.1f, 1.0f,  10.0f, "%.1f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Move Speed",   &enemyParams_.moveSpeed,   0.1f, 0.5f,  20.0f, "%.1f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Min Distance", &enemyParams_.minDistance, 0.1f, 0.0f,   5.0f, "%.2f");

        ImGui::Separator();

        // Combat
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Combat");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Invincible Duration", &enemyParams_.invincibleDuration, 0.05f, 0.1f, 2.0f, "%.2fs");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Knockback Decay", &enemyParams_.knockbackDecay, 0.05f, 0.1f, 1.0f, "%.2f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Death Duration", &enemyParams_.deathDuration, 0.05f, 0.1f, 3.0f, "%.2fs");

        ImGui::Separator();

        // Dash
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Dash");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Charge Time", &dashParams_.chargeMaxTime, 0.1f, 0.1f, 5.0f, "%.2fs");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Dash Duration", &dashParams_.duration, 0.05f, 0.1f, 2.0f, "%.2fs");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Dash Speed", &dashParams_.speed, 0.5f, 5.0f, 50.0f, "%.1f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Trigger Interval", &dashParams_.triggerInterval, 0.1f, 0.5f, 10.0f, "%.2fs");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        float chancePercent = dashParams_.triggerChance * 100.0f;
        if (ImGui::SliderFloat("Trigger Chance", &chancePercent, 0.0f, 100.0f, "%.0f%%")) {
            dashParams_.triggerChance = chancePercent / 100.0f;
        }

        ImGui::Spacing();
        ImGui::Text("Prediction Line");
        ImGui::Indent();
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Enemy Offset", &dashParams_.enemyOffset, 0.1f, 0.0f, 5.0f, "%.2f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Line Length", &dashParams_.lineLength, 0.05f, 0.1f, 2.0f, "%.2f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Line Width", &dashParams_.lineWidth, 0.1f, 0.5f, 5.0f, "%.2f");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::DragFloat("Center Offset", &dashParams_.lineCenterOffset, 0.05f, 0.0f, 1.0f, "%.2f");
        ImGui::Unindent();

        ImGui::Separator();

        if (ImGui::Button("Save to JSON", ImVec2(-1, 0))) SaveParams();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Assets/Data/EnemyParams.json");
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Enemy Details")) {
        int index = 0;
        for (const auto& enemy : enemies_) {
            ImGui::PushID(index);
            ImGui::Text("Enemy #%d %s", index, enemy->IsDead() ? "[Dying]" : "");
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

void Enemies::Spawn() {
    if (done_) return;
    if (!target_) return;
    if (status_.maxEnemyCount <= enemies_.size()) return;

    Vector3 position = target_->GetPosition();
    position += Vector3::Random().Normalize() * MathUtils::Random(distance_.x, distance_.y);
    position.y = .5f;

    auto enemy = std::make_unique<Enemy>(enemyParams_, position);
    enemy->SetParticleSystem(particle_);
    enemy->SetTarget(target_);

    auto dash = std::make_unique<DashActionModule>();
    dash->SetParams(&dashParams_);
    enemy->SetActionModule(std::move(dash));

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

    // ファイルやキーが無いときは、Params の初期値のまま使う
    if (!json->Load("EnemyParams")) return;

    json->TryGetValue("EnemyParams", "Basic", "MaxHp", enemyParams_.maxHp);
    json->TryGetValue("EnemyParams", "Basic", "MoveSpeed", enemyParams_.moveSpeed);
    json->TryGetValue("EnemyParams", "Basic", "MinDistance", enemyParams_.minDistance);
    json->TryGetValue("EnemyParams", "Basic", "SpawnDistance", distance_);
    json->TryGetValue("EnemyParams", "Invincible", "Duration", enemyParams_.invincibleDuration);
    json->TryGetValue("EnemyParams", "Knockback", "Decay", enemyParams_.knockbackDecay);
    json->TryGetValue("EnemyParams", "Death", "Duration", enemyParams_.deathDuration);

    json->TryGetValue("EnemyParams", "Dash", "ChargeMaxTime", dashParams_.chargeMaxTime);
    json->TryGetValue("EnemyParams", "Dash", "Duration", dashParams_.duration);
    json->TryGetValue("EnemyParams", "Dash", "Speed", dashParams_.speed);
    json->TryGetValue("EnemyParams", "Dash", "TriggerInterval", dashParams_.triggerInterval);
    json->TryGetValue("EnemyParams", "Dash", "TriggerChance", dashParams_.triggerChance);
    json->TryGetValue("EnemyParams", "Dash", "EnemyOffset", dashParams_.enemyOffset);
    json->TryGetValue("EnemyParams", "Dash", "LineLength", dashParams_.lineLength);
    json->TryGetValue("EnemyParams", "Dash", "LineWidth", dashParams_.lineWidth);
    json->TryGetValue("EnemyParams", "Dash", "LineCenterOffset", dashParams_.lineCenterOffset);
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
    json->SetValue("EnemyParams", "Dash", "ChargeMaxTime", dashParams_.chargeMaxTime);
    json->SetValue("EnemyParams", "Dash", "Duration", dashParams_.duration);
    json->SetValue("EnemyParams", "Dash", "Speed", dashParams_.speed);
    json->SetValue("EnemyParams", "Dash", "TriggerInterval", dashParams_.triggerInterval);
    json->SetValue("EnemyParams", "Dash", "TriggerChance", dashParams_.triggerChance);
    json->SetValue("EnemyParams", "Dash", "EnemyOffset", dashParams_.enemyOffset);
    json->SetValue("EnemyParams", "Dash", "LineLength", dashParams_.lineLength);
    json->SetValue("EnemyParams", "Dash", "LineWidth", dashParams_.lineWidth);
    json->SetValue("EnemyParams", "Dash", "LineCenterOffset", dashParams_.lineCenterOffset);

    json->Save("EnemyParams");
}
