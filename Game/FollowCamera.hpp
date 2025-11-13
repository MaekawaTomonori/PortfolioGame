#ifndef CAMERA_CONTROLLER_HPP_
#define CAMERA_CONTROLLER_HPP_

#include "Input.hpp"
#include "Math/Vector3.hpp"
#include "Camera/Controller/CameraController.hpp"

class GameObject;

class FollowCamera {
    GameObject* target_;
    Input* input_;
    CameraController* cameraManager_;
    DebugUI* debug_ = nullptr;
    
    Vector3 offset_{0.0f, 0.0f, 0.0f};
    float yaw_ = 0.f, pitch_ = 0.f;

    bool active_ = false;

    Vector3 shake = {};
    float shakeTimer_ = 0.f;
    float shakePower_ = 0.f;

public:
    FollowCamera();
    ~FollowCamera();
    
    void Initialize();
    void Update();

    void SetActive(bool _state);
    void SetDebug(DebugUI* _debug) { debug_ = _debug; }

    // Target設定
    void SetTarget(GameObject* target) { target_ = target; }

    void Shake(float _time, float _power);

private:
    void Load();
    void Save();
    void Debug();

    void UpdateCameraPosition() const;
    void ApplyShake();
};

#endif // CAMERA_CONTROLLER_HPP_