#ifndef MOVEMENT_CONTEXT_HPP_
#define MOVEMENT_CONTEXT_HPP_

#include "Math/Vector3.hpp"

class GameObject;

struct MovementContext {
    Vector3 position = {};
    Vector3 velocity = {};
    Vector3 rotation = {};
    Vector3 moveDirection = {};

    bool isDashRequested = false;
    
    GameObject* owner = nullptr;
    GameObject* target = nullptr;

    void Reset() {
        position = {};
        velocity = {};
        rotation = {};
        moveDirection = {};
        isDashRequested = false;
        owner = nullptr;
        target = nullptr;
    }
};

#endif // MOVEMENT_CONTEXT_HPP_
