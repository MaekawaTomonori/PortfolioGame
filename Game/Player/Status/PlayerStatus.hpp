#ifndef PLAYER_STATUS_HPP_
#define PLAYER_STATUS_HPP_

struct PlayerStatus {
    float hp = 10.f;
    float damage = 1.f;
    float ms = 1.f;
    float as = 1.f;

    // TODO: 攻撃の種類（通常/貫通/範囲など）を追加し、JSONで保存・復元できるようにする
    // TODO: スキルレベル（BlackHoleなど各スキルのLv）をここまたはGameStatusに持たせ、
    //       SkillManagerのJSON保存に組み込む
}; // struct PlayerStatus

#endif // PLAYER_STATUS_HPP_
