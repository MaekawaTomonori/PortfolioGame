#ifndef MOVEMENT_CONTEXT_HPP_
#define MOVEMENT_CONTEXT_HPP_

#include "Math/Vector3.hpp"

class GameObject;

struct MovementContext {
    Vector3 position = {};
    Vector3 velocity = {};
    Vector3 rotation = {};
    Vector3 targetPosition = {};
    Vector3 moveDirection = {};

    bool isDashRequested = false;
    bool isJumpRequested = false;
    bool isFlashRequested = false;

    GameObject* owner = nullptr;
    GameObject* target = nullptr;

    void Reset() {
        position = {};
        velocity = {};
        rotation = {};
        targetPosition = {};
        moveDirection = {};
        isDashRequested = false;
        isJumpRequested = false;
        isFlashRequested = false;
        owner = nullptr;
        target = nullptr;
    }
};

#endif // MOVEMENT_CONTEXT_HPP_
