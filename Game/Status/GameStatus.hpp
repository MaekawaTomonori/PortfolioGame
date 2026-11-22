#ifndef GameStatus_HPP_
#define GameStatus_HPP_
#include <cstdint>

#include "Player/Status/PlayerStatus.hpp"

struct GameStatus {
    uint32_t point = 0;
    float time = 15.f;
    uint16_t requirementKill = 5;
    uint16_t maxEnemyCount = 100;
    float enemySpawnInterval = 2.f;
    PlayerStatus playerStatus;

    /** @brief GameStatus.json から読む。無い値は今の値のまま */
    void Load();
    void Save() const;
};

#endif // GameStatus_HPP_
