#ifndef COLLISION_ATTRIBUTE_HPP_
#define COLLISION_ATTRIBUTE_HPP_

enum class CollisionAttribute {
    None = 0,
    Player = 0b1,
    Enemy = 0b1 << 1,
    PlayerBullet = 0b1 << 2,
    Skill = 0b1 << 3,
    EnemyBullet = 0b1 << 4,
    Damage = 0b1 << 5,

};

#endif // COLLISION_ATTRIBUTE_HPP_
