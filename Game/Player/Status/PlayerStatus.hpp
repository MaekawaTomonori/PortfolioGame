#ifndef PlayerStatus_HPP_
#define PlayerStatus_HPP_

struct PlayerStatus {
    float hp;
    float damage;
    float ms;
    float as;

    // TODO: 攻撃の種類（通常/貫通/範囲など）を追加し、JSONで保存・復元できるようにする
    // TODO: スキルレベル（BlackHoleなど各スキルのLv）をここまたはGameStatusに持たせ、
    //       SkillManagerのJSON保存に組み込む
}; // struct PlayerStatus

#endif // PlayerStatus_HPP_
