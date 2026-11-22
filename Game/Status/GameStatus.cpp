#include "GameStatus.hpp"

#include <variant>
#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"

namespace {
    constexpr const char* FILE_NAME = "GameStatus";

    /** @brief 無い、または型が違うときは _out をそのままにする */
    template <typename T>
    void Read(const char* _group, const char* _key, T& _out) {
        const auto value = Singleton<JsonParams>::GetInstance()->GetValue(FILE_NAME, _group, _key);
        if (const T* found = std::get_if<T>(&value)) _out = *found;
    }

    /** @brief JSON では整数を int32_t で持つので、読んでから uint16_t に直す */
    void ReadCount(const char* _group, const char* _key, uint16_t& _out) {
        int32_t count = _out;
        Read(_group, _key, count);
        _out = static_cast<uint16_t>(count);
    }
}

void GameStatus::Load() {
    if (!Singleton<JsonParams>::GetInstance()->Load(FILE_NAME)) return;

    Read("Stage", "Time", time);
    ReadCount("Stage", "MaxEnemyCount", maxEnemyCount);
    ReadCount("Stage", "RequirementKill", requirementKill);
    Read("Stage", "EnemySpawnInterval", enemySpawnInterval);
    Read("Player", "HP", playerStatus.hp);
    Read("Player", "Damage", playerStatus.damage);
    Read("Player", "MS", playerStatus.ms);
    Read("Player", "AS", playerStatus.as);
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
