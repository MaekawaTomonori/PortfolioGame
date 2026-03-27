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

    // TODO: スキルレベルテーブルを追加し、GameScene::SaveStatus / LoadStatus で
    //       JSONに永続化できるようにする（例: std::map<std::string, int> skillLevels）
};

#endif // GameStatus_HPP_
