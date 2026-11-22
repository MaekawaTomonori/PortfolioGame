#include "GameStatus.hpp"

#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"

namespace {
    constexpr const char* FILE_NAME = "GameStatus";

    /** @brief JSON では整数を int32_t で持つので、読んでから uint16_t に直す */
    void ReadCount(const JsonParams& _json, const char* _group, const char* _key, uint16_t& _out) {
        int32_t count = _out;
        if (_json.TryGetValue(FILE_NAME, _group, _key, count)) _out = static_cast<uint16_t>(count);
    }
}

void GameStatus::Load() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    if (!json->Load(FILE_NAME)) return;

    json->TryGetValue(FILE_NAME, "Stage", "Time", time);
    ReadCount(*json, "Stage", "MaxEnemyCount", maxEnemyCount);
    ReadCount(*json, "Stage", "RequirementKill", requirementKill);
    json->TryGetValue(FILE_NAME, "Stage", "EnemySpawnInterval", enemySpawnInterval);
    json->TryGetValue(FILE_NAME, "Player", "HP", playerStatus.hp);
    json->TryGetValue(FILE_NAME, "Player", "Damage", playerStatus.damage);
    json->TryGetValue(FILE_NAME, "Player", "MS", playerStatus.ms);
    json->TryGetValue(FILE_NAME, "Player", "AS", playerStatus.as);
}

void GameStatus::Save() const {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->SetValue(FILE_NAME, "Stage", "Time", time);
    json->SetValue(FILE_NAME, "Stage", "MaxEnemyCount", static_cast<int32_t>(maxEnemyCount));
    json->SetValue(FILE_NAME, "Stage", "RequirementKill", static_cast<int32_t>(requirementKill));
    json->SetValue(FILE_NAME, "Stage", "EnemySpawnInterval", enemySpawnInterval);
    json->SetValue(FILE_NAME, "Player", "HP", playerStatus.hp);
    json->SetValue(FILE_NAME, "Player", "Damage", playerStatus.damage);
    json->SetValue(FILE_NAME, "Player", "MS", playerStatus.ms);
    json->SetValue(FILE_NAME, "Player", "AS", playerStatus.as);
    json->Save(FILE_NAME);
}
