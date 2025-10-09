#ifndef BEHAVIOR_COMPONENT_HPP_
#define BEHAVIOR_COMPONENT_HPP_

#include "Math/Vector3.hpp"

class GameObject;

class BehaviorComponent {
private:
    GameObject* owner_;
    GameObject* target_;
    
public:
    BehaviorComponent();
    ~BehaviorComponent();
    
    void Initialize(GameObject* owner);
    Vector3 Update(float deltaTime);
    
    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }
};

#endif // BEHAVIOR_COMPONENT_HPP_