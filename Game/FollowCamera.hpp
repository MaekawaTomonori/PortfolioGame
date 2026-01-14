#ifndef CAMERA_CONTROLLER_HPP_
#define CAMERA_CONTROLLER_HPP_

#include "Math/Vector3.hpp"
#include "Camera/Controller/CameraController.hpp"

class GameObject;
class Enemies;

class FollowCamera {
    CameraController* cameraController_;
    
    GameObject* target_ = nullptr;
    Enemies* enemies_ = nullptr;

    Vector3 offset_ {};

    // カメラパラメータ（角度と距離で表現）
    float distance_ = 15.f;           // カメラとターゲットの距離
    float yaw_ = 0.f;                 // 水平角度（ラジアン）
    float pitch_ = -0.785f;           // 垂直角度（ラジアン、デフォルト-45度：斜め上から見下ろす）
    float interpolationTime_ = 0.1f;                  // 補間係数

    // 動的ズーム設定
    float minDistance_ = 12.f;  // 最小距離
    float maxDistance_ = 35.f;  // 最大距離
    float zoomSensitivity_ = 0.8f;    // ズーム感度

    bool active_ = false;

    bool shaking_ = false;
    Vector3 shake = {};
    float shakeTimer_ = 0.f;
    float shakePower_ = 0.f;

public:
    FollowCamera();
    ~FollowCamera();
    
    void Initialize();
    void Update();
    void Debug();

    void SetActive(bool _state);

    // Target設定
    void SetTarget(GameObject* _target) { target_ = _target; }
    void SetEnemies(Enemies* _enemies) { enemies_ = _enemies; }

    const Camera* GetCamera() const { return cameraController_ ? cameraController_->GetActive() : nullptr; }

    void Shake(float _time, float _power);

private:
    void Load();
    void Save();

    void UpdateCameraDistance();
    void UpdateCameraPosition();
    void ApplyShake();
};

#endif // CAMERA_CONTROLLER_HPP_