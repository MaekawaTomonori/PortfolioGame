#ifndef COLLISION_TYPE_HPP_
#define COLLISION_TYPE_HPP_

enum class CollisionType {
    None = 0,
    Player = 0b1,
    Enemy = 0b1 << 1,
    P_Bullet = 0b1 << 2,

};

#endif 
