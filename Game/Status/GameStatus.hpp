#ifndef GameStatus_HPP_
#define GameStatus_HPP_
#include <cstdint>

#include "Player/Status/PlayerStatus.hpp"

struct GameStatus {
    uint32_t point;
    float time;
    uint16_t requirementKill;
    uint16_t maxEnemyCount;
    float enemySpawnInterval;
    PlayerStatus playerStatus;
};

#endif // GameStatus_HPP_
